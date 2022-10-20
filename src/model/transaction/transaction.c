#include "transaction.h"

#include "utils/sys_utils.h"

GHashTable *global_transaction_table;
GHashTable *utxo;


/**
 * Setup the transaction system, including setting up:
 * 1. The global transaction table.
 * 2. Unspent Transaction Output (UTXO)
 */
void initialize_transaction_system() {
    global_transaction_table = g_hash_table_new(g_str_hash, g_str_equal);
    utxo = g_hash_table_new(g_str_hash, g_str_equal);
}

void get_all_transaction(){
    GList* list_values= g_hash_table_get_values(global_transaction_table);
    printf("There are %d transactions in the system\n", g_list_length(list_values));
    for(int i=0;i< g_list_length(list_values);i++){
        transaction *t= (transaction *)(g_list_nth_data(list_values,i));
        printf("Transaction %d, have input count: %d, have output count: %d, have version number: %d\n", get_transaction_txid(t),t->tx_in_count,t->tx_out_count,t->version);
    }
}

/**
 * Get the hash code of a transaction.
 * @param transaction
 * @return TXID
 */
TXID get_transaction_txid(transaction *transaction) { return g_direct_hash(transaction); }

/**
 * Convert TXID to string.
 * @param id TXID
 * @return TXID in string.
 */
char *convert_txid_to_str(TXID id) {
    char *res = (char *)malloc(8);
    sprintf(res, "%x", id);
    return res;
}

/**
 * Retrieve the target public key from a transaction output.
 * @param output
 * @return The public key buried in the output.
 */
public_key get_transaction_output_public_key(transaction_output *output) { return output->pk_script; }

/**
 * Check if the transaction's format is valid. Essentially, the sum of
 * outputs must be smaller than that of the input.
 * @param t A transaction.
 * @return true for valid, and false otherwise.
 */
bool check_transaction_format(transaction *t) {
    long long int output_sum = 0, input_sum = 0;
    transaction_output **outputs = t->tx_outs;
    transaction_input **inputs = t->tx_ins;
    unsigned int number_of_inputs = t->tx_in_count;
    unsigned int number_of_outputs = t->tx_out_count;

    if (number_of_inputs == 0) {
        if (VERBOSE) printf("Number of inputs of a t cannot be 0.\n");
        return false;
    }

    if (number_of_outputs == 0) {
        if (VERBOSE) printf("Number of outputs of a t cannot be 0.\n");
        return false;
    }

    for (int i = 0; i < number_of_inputs; i++) {
        transaction_input *current_input = inputs[i];
        transaction_outpoint *outpoint = current_input->outpoint;
        unsigned int outpoint_output_idx = outpoint->index;

        char *txid_str = convert_txid_to_str(outpoint->hash);
        if (!g_hash_table_contains(global_transaction_table, txid_str)) {
            if (VERBOSE) printf("Input does not refer to an existing t. Input TXID: %s\n", txid_str);
            free(txid_str);
            return false;
        }

        transaction *outpoint_transaction = g_hash_table_lookup(global_transaction_table, txid_str);
        if (outpoint_output_idx <= outpoint_transaction->tx_out_count) {
            if (VERBOSE) printf("Input outpoint index out of bounds. Input outpoint index: %u\n", outpoint_output_idx);
            return false;
        }

        input_sum += outpoint_transaction->tx_outs[outpoint_output_idx]->value;
        free(txid_str);
    }

    for (int i = 0; i < number_of_outputs; i++) {
        transaction_output *current_output = t->tx_outs[i];
        output_sum += current_output->value;
    }

    if (input_sum != output_sum) {
        if (VERBOSE)
            printf("Input sums (%llu) values must be equal to the output sums (%llu) in a transaction.\n", input_sum,
                   output_sum);
        return false;
    }

    return true;
}

/**
 * Add a transaction to the system.
 * @param t A transaction.
 * @param skip_format_checking Choose whether to skip format checking.
 * @return 0 for success, other numbers indicate failure.
 */
int register_transaction_in_system(transaction *t, bool skip_format_checking) {

    if (!skip_format_checking && !check_transaction_format(t)) return -1;


    TXID transaction_id = get_transaction_txid(t);




    char *txid_str = convert_txid_to_str(transaction_id);
    g_hash_table_insert(global_transaction_table, txid_str, t);


//    for (int i = 0; i < t->tx_in_count; i++) {
//        transaction_outpoint *outpoint = t->tx_ins[i]->outpoint;
//
//        unsigned int outpoint_output_idx = outpoint->index;
//        char *input_txid_str = convert_txid_to_str(outpoint->hash);
//        transaction_output *outpoint_output =
//            ((transaction *)g_hash_table_lookup(global_transaction_table, input_txid_str))
//                ->tx_outs[outpoint_output_idx];
//
//
//        public_key previous_transaction_public_key = get_transaction_output_public_key(outpoint_output);
//
//        free(g_hash_table_lookup(global_transaction_table, previous_transaction_public_key));
//        g_hash_table_remove(global_transaction_table, previous_transaction_public_key);
//        free(input_txid_str);
//    }
//
//
//
//    for (int i = 0; i < t->tx_out_count; i++) {
//        transaction_output *current_output = t->tx_outs[i];
//        public_key current_output_public_key = get_transaction_output_public_key(current_output);
//        if (g_hash_table_contains(global_transaction_table, current_output_public_key))
//            *((long long int *)g_hash_table_lookup(global_transaction_table, current_output_public_key)) +=
//                current_output->value;
//        else {
//            long long int* balance = (long long int*)malloc(sizeof(long long int));
//            *balance = current_output->value;
//            g_hash_table_insert(global_transaction_table, current_output_public_key, balance);
//        }
//    }

    return 0;
}



int register_coin_pool() {
    transaction* coin_pool= (transaction *)malloc(sizeof (transaction));
    coin_pool->tx_in_count=0;
    coin_pool->tx_out_count=1;
    coin_pool->version=222;
    transaction_output **tx_outs= malloc(sizeof(transaction_output)*1);
    transaction_output *output=malloc(sizeof(transaction_output)*1);;
    output->value=TOTAL_COIN_NUMBER;
    tx_outs[0]=output;

    coin_pool->tx_outs=tx_outs;


    char *txid_str = convert_txid_to_str(POOL_TXID);
    g_hash_table_insert(global_transaction_table, txid_str, coin_pool);

    return 0;
}

transaction* create_new_transaction(){
    transaction* t= (transaction *)malloc(sizeof(transaction));
    t->tx_out_count=0;
    t->tx_in_count=0;
    transaction_input **tx_ins= malloc(MAXIMUM_INPUT_PER_TX*sizeof(transaction_input));
    transaction_output **tx_outs=malloc(MAXIMUM_OUTPUT_PER_TX*sizeof(transaction_output));
    t->tx_outs=tx_outs;
    t->tx_ins=tx_ins;
    return t;
}

//transaction* find_transaction(TXID txid){
//    char *txid_str = convert_txid_to_str(txid);
//    transaction tg_hash_table_lookup(global_transaction_table,txid_str);
//}

int transaction_receive_coin(transaction *t, transaction_outpoint* outpoint){
    //Insert an input for the transaction
    transaction_input *input=malloc(sizeof(transaction_input));
    input->outpoint=outpoint;
    for(int i=0;i<MAXIMUM_INPUT_PER_TX;i++){
        if(t->tx_ins[i]==NULL){
            t->tx_ins[i]=input;
            t->tx_in_count++;
            return 0;
        }
    }
    return 1;
}

/**
 * Add an output for a transaction. It is used to send coins to other transaction.
 * @param t
 * @param value
 * @return The index of output in its transaction's output array
 */
int transaction_send_coin(transaction *t, long int value){
    transaction_output *output= malloc(sizeof(transaction_output));
    output->value=value;

    for(int i=0;i<MAXIMUM_OUTPUT_PER_TX;i++){
        if(t->tx_outs[i]==NULL){
            t->tx_outs[i]=output;
            t->tx_out_count++;
            return i;
        }
    }
    return -1;
}

int print_UTXO(){
    GList* list= g_hash_table_get_values(global_transaction_table);
    for(int i=0;i< g_list_length(list);i++){
        transaction *t= (transaction *)(g_list_nth_data(list,i));
        long int coin_receive=0;
        long int coin_spent=0;
        for(int i=0;i<t->tx_in_count;i++){
            transaction_outpoint *outpoint=t->tx_ins[i]->outpoint;
            transaction* transaction_in= (transaction *)g_hash_table_lookup(global_transaction_table, convert_txid_to_str(outpoint->hash));
            coin_receive+=transaction_in->tx_outs[outpoint->index]->value;
        }
        for(int i=0;i<t->tx_out_count;i++){
            coin_spent+=t->tx_outs[i]->value;
        }
        printf("Transaction %d, receive %d coins, spent %d coins, balance is: %d coins\n", get_transaction_txid(t),coin_receive,coin_spent,coin_receive-coin_spent);
    }
    return 0;

}
