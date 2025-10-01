#include <array>
#include <cstdint>
#include <vector>

using Hash256 = std::array<std::uint8_t, 32>;
using Script = std::vector<std::uint8_t>;

struct TxOutPt
{
    const Hash256 hash;         // The transaction ID (TXID) of the transaction holding the output to spend.
    const std::uint32_t index;  // The output index of the specific output to spend from the transaction.
};

struct TxIn
{
    const TxOutPt previous_outpoint;  // The previous outpoint being spent.
    const std::uint32_t sequence;     // Sequence number.
    const Script signature_script;    // A script-language script.

    explicit TxIn(Hash256 prev_hash, std::uint32_t prev_idx, std::uint32_t seq, Script &&sig_script)
        : previous_outpoint{prev_hash, prev_idx},
          sequence{seq},
          signature_script{std::move(sig_script)}
    {
    }
};

struct TxOut
{
    const std::uint64_t value;  // Number of crypto to spend.
    const Script pk_script;     // Defines the conditions which must be met to spend this output.

    explicit TxOut(std::uint64_t val, Script &&pk_scr) : value{val}, pk_script{std::move(pk_scr)} {}
};

class Tx
{
   public:
    const std::uint32_t version;       // Transaction version number. Default is 1.
    const std::uint32_t lock_time;     // A time number.
    const std::vector<TxIn> tx_ins;    // Array of transaction inputs.
    const std::vector<TxOut> tx_outs;  // Array of transaction outputs.

    Tx() = delete;
    explicit Tx(std::uint32_t version,
                std::uint32_t lock_time,
                std::vector<TxIn> &&tx_ins,
                std::vector<TxOut> &&tx_outs)
        : version{version},
          lock_time{lock_time},
          tx_ins{std::move(tx_ins)},
          tx_outs{std::move(tx_outs)}
    {
    }
};