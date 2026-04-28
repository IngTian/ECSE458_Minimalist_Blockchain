#include "log_utils.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>

#include "../model/block/block.h"
#include "../model/transaction/transaction.h"
#include "constants.h"
#include "cryptography.h"
#include "logger.hpp"
#include "sys_utils.h"

// Regular text
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

// Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

// Regular underline text
#define UBLK "\e[4;30m"
#define URED "\e[4;31m"
#define UGRN "\e[4;32m"
#define UYEL "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define UWHT "\e[4;37m"

// Regular background
#define BLKB "\e[40m"
#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YELB "\e[43m"
#define BLUB "\e[44m"
#define MAGB "\e[45m"
#define CYNB "\e[46m"
#define WHTB "\e[47m"

// High intensity background
#define BLKHB "\e[0;100m"
#define REDHB "\e[0;101m"
#define GRNHB "\e[0;102m"
#define YELHB "\e[0;103m"
#define BLUHB "\e[0;104m"
#define MAGHB "\e[0;105m"
#define CYNHB "\e[0;106m"
#define WHTHB "\e[0;107m"

// High intensity text
#define HBLK "\e[0;90m"
#define HRED "\e[0;91m"
#define HGRN "\e[0;92m"
#define HYEL "\e[0;93m"
#define HBLU "\e[0;94m"
#define HMAG "\e[0;95m"
#define HCYN "\e[0;96m"
#define HWHT "\e[0;97m"

// Bold high intensity text
#define BHBLK "\e[1;90m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHBLU "\e[1;94m"
#define BHMAG "\e[1;95m"
#define BHCYN "\e[1;96m"
#define BHWHT "\e[1;97m"

// Reset
#define reset "\e[0m"

/**
 * Convert a char array of specified length to a string
 * in hexadecimal format, for printing and logging.
 * @param ptr Pointer to the char array.
 * @param byte_length The length of the byte array.
 * @return The corresponding string in hexadecimal format.
 */
char *convert_char_hexadecimal(char *ptr, unsigned int byte_length) {
    char *ret_val = (char *)malloc(2 * byte_length + 1);
    char *ret_val_counter = ret_val;
    for (int i = 0; i < byte_length; i++) {
        sprintf(ret_val_counter, "%02hhX", *ptr++);
        ret_val_counter += 2;
    }
    *ret_val_counter = '\0';
    return ret_val;
}

/**
 * Log via spdlog. Backward-compatible printf-style interface.
 * For new code prefer bc::log::info(scope, "{}", ...) from logger.hpp.
 * @param scope The topic of the log.
 * @param log_level The log level (LOG_DEBUG, LOG_INFO, LOG_ERROR).
 * @param format printf-style format string.
 * @param ... Variadic arguments matching format.
 */
void general_log(char *scope, int log_level, char *format, ...) {
    if (!VERBOSE || log_level < LOG_LEVEL) return;

    bc::log::ensure_initialized();
    auto level = bc::log::detail::level_from_legacy(log_level);
    if (!spdlog::default_logger_raw()->should_log(level)) return;

    // vsnprintf into a stack buffer first; spill to heap if message is huge
    // (typical messages here are short — UTXO hashes, error strings).
    char stack_buf[1024];
    va_list args;
    va_start(args, format);
    int needed = vsnprintf(stack_buf, sizeof(stack_buf), format, args);
    va_end(args);

    if (needed < 0) return;  // formatting error; just drop

    if (static_cast<size_t>(needed) < sizeof(stack_buf)) {
        spdlog::default_logger_raw()->log(level, "({}) ----> {}", scope, stack_buf);
    } else {
        std::string heap_buf(static_cast<size_t>(needed) + 1, '\0');
        va_list args2;
        va_start(args2, format);
        vsnprintf(heap_buf.data(), heap_buf.size(), format, args2);
        va_end(args2);
        heap_buf.resize(static_cast<size_t>(needed));
        spdlog::default_logger_raw()->log(level, "({}) ----> {}", scope, heap_buf);
    }
}

namespace bc::log {

void ensure_initialized() {
    static std::once_flag init_flag;
    std::call_once(init_flag, [] {
        auto logger = spdlog::stdout_color_mt("blockchain");
        spdlog::set_default_logger(logger);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        // Map utils/constants.h LOG_LEVEL to spdlog level.
        switch (LOG_LEVEL) {
            case 0: spdlog::set_level(spdlog::level::debug); break;
            case 1: spdlog::set_level(spdlog::level::info); break;
            case 2: spdlog::set_level(spdlog::level::err); break;
            default: spdlog::set_level(spdlog::level::info); break;
        }
    });
}

}  // namespace bc::log

/**
 * Generate a .dot file to represent our system.
 * @param block_list A list of blocks.
 * @param filename The filename to save the .dot file.
 * @author Luke E
 */
void generate_dot_representation(block **block_list, int list_len, char *filename) {
    FILE *fp = fopen(filename, "w");
    fprintf(fp, "digraph G {\n");
    fprintf(fp, "compound=true;\n");
    fprintf(fp, "node [shape=record];\n");

    if (list_len == 1) {
        fprintf(fp, "Block0\n");
        fprintf(fp, "}\n");
        fclose(fp);
        return;
    }

    // Create sub-graphs(blocks&transactions)
    for (int i = 0; i < list_len; i++) {
        fprintf(fp, "subgraph cluster_%d{\n ", i);
        fprintf(fp, "label = \"Block%d\";\n", i);
        fprintf(fp, "DUMMY_%d [shape=point style=invis];\n", i);
        for (int j = 0; j < block_list[i]->txn_count; j++) {
            uint8_t *txid_bin = get_transaction_txid(block_list[i]->txns[j]);
            char *txid_dot = hash_to_hex(txid_bin);
            free(txid_bin);
            fprintf(fp, "txid%s;\n", txid_dot);
            free(txid_dot);
        }
        fprintf(fp, "}\n");
    }

    // Connect all blocks
    for (int b = 0; b < list_len - 1; b++) {
        int a = b + 1;
        fprintf(fp, "DUMMY_%d -> DUMMY_%d [ltail=cluster_%d,lhead=cluster_%d];\n", b, a, b, a);
    }

    // Connect all transactions
    static const uint8_t zero_hash[32] = {0};
    for (int m = 0; m < list_len; m++) {
        for (int n = 0; n < block_list[m]->txn_count; n++) {
            uint8_t *txid_bin = get_transaction_txid(block_list[m]->txns[n]);
            char *txid_trans = hash_to_hex(txid_bin);
            free(txid_bin);
            for (int o = 0; o < block_list[m]->txns[n]->tx_in_count; o++) {
                const uint8_t *prev_hash = block_list[m]->txns[n]->tx_ins[o].previous_outpoint.hash;
                if (memcmp(prev_hash, zero_hash, 32) != 0) {
                    char *txid_previous = hash_to_hex(prev_hash);
                    fprintf(fp, "txid%s -> txid%s;\n", txid_previous, txid_trans);
                    free(txid_previous);
                }
            }
            free(txid_trans);
        }
    }

    fprintf(fp, "}\n");
    fclose(fp);
    return;
}

/**
 * Print a series of bytes
 * in hexadecimal format.
 * @param data Incoming data.
 * @param size The size of the incoming data in bytes.
 */
void print_hex(unsigned char *data, int size) {
    size_t i;
    printf("0x");
    for (i = 0; i < size; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * Write log to a file.
 * @param dir A file number.
 * @param mode Writing mode.
 * @param lines Lines.
 * @param line_size The number of lines.
 */
void write_to_file(char *dir, char *mode, char **lines, int line_size) {
    FILE *fptr;
    fptr = fopen(dir, mode);
    for (int i = 0; i < line_size; ++i) {
        fprintf(fptr, "%s\n", lines[i]);
    }
    fclose(fptr);
}
