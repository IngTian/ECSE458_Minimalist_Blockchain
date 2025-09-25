#include "log_utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/constants.h"
#include "utils/sys_utils.h"

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
 * Log.
 * @param scope The topic of the log.
 * @param log_level The log level. (LOG_DEBUG, LOG_INFO, LOG_ERROR)
 * @param format
 * @param ... Messages, similar to printf.
 */
void general_log(char *scope, int log_level, char *format, ...) {
    if (!VERBOSE || log_level < LOG_LEVEL) return;

    // Print marcos.
    char *curr_time = get_str_timestamp(LOG_TIME_FORMAT, LOG_TIME_LENGTH);

    if (log_level == LOG_DEBUG)
        printf(BYEL "%s" reset, "DBUG");
    else if (log_level == LOG_INFO)
        printf(BBLU "%s" reset, "INFO");
    else if (log_level == LOG_ERROR)
        printf(BRED "%s" reset, "FATA");
    else
        return;

    printf(CYN "[%s]" reset, curr_time);
    free(curr_time);

    printf("(%s)", scope);
    printf(" ----> ");

    // Print custom message.
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);

    printf("\n");
}

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
        char *txid_dot;
        fprintf(fp, "subgraph cluster_%d{\n ", i);
        fprintf(fp, "label = \"Block%d\";\n", i);
        fprintf(fp, "DUMMY_%d [shape=point style=invis];\n", i);
        for (int j = 0; j < block_list[i]->txn_count; j++) {
            txid_dot = get_transaction_txid(block_list[i]->txns[j]);
            fprintf(fp, "txid%s;\n", txid_dot);
        }
        fprintf(fp, "}\n");
    }

    // Connect all blocks
    for (int b = 0; b < list_len - 1; b++) {
        int a = b + 1;
        fprintf(fp, "DUMMY_%d -> DUMMY_%d [ltail=cluster_%d,lhead=cluster_%d];\n", b, a, b, a);
    }

    // Connect all transactions
    for (int m = 0; m < list_len; m++) {
        char *txid_trans;
        char *txid_previous;
        for (int n = 0; n < block_list[m]->txn_count; n++) {
            txid_trans = get_transaction_txid(block_list[m]->txns[n]);
            for (int o = 0; o < block_list[m]->txns[n]->tx_in_count; o++) {
                txid_previous = block_list[m]->txns[n]->tx_ins[o].previous_outpoint.hash;
                if (strlen(txid_previous) > 0) {
                    fprintf(fp, "txid%s -> txid%s;\n", txid_previous, txid_trans);
                }
            }
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
