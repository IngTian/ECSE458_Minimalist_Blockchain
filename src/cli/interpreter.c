#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../model/transaction/transaction.h"
#include "../model/transaction/transaction_persistence.h"
#include "../model/block/block.h"
#include "../utils/log_utils.h"
#include "../utils/mjson.h"
#include "shell.h"

secp256k1_pubkey *public_key_temp;
secp256k1_ecdsa_signature *signature_temp;

/*
 * -----------------------------------------------------------
 * System functions
 * -----------------------------------------------------------
 */
int help();
int quit();
int bad_command();
int bad_command_file_does_not_exist();

/*
 * -----------------------------------------------------------
 * Blockchain functions
 * -----------------------------------------------------------
 */
int init();
int run(char *script);

int cli_transaction_list_all_transactions();
int cli_transaction_add_transaction_to_system(char *script);
int cli_transaction_list_utxo();
int cli_transaction_get_genesis_transaction_private_key();

int cli_cryptography_create_private_key();
int cli_cryptography_create_public_key(char *private_key);
int cli_cryptography_sign(char *private_key, char *msg_hash);
int cli_cryptography_verify(char *public_key, char *msg_hash, char *signature);

int cli_block_list_all_blocks();
int cli_block_add_block_to_system(char *script);
int cli_block_get_genesis_block_hash();

/**
 * Interpret and execute the input commands.
 * @param command_args Input command arguments.
 * @param args_size Number of input arguments.
 * @return Return 0 if the command runs successfully. If the input command is not defined, return 1.
 * If the input command with the argument file and encounter the file not exist exception, return 2.
 */
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1) return bad_command();

    for (i = 0; i < args_size; i++) {  // strip spaces new line etc
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        if (args_size != 1) return bad_command();
        return help();
    } else if (strcmp(command_args[0], "quit") == 0) {
        if (args_size != 1) return bad_command();
        return quit();
    } else if (strcmp(command_args[0], "run") == 0) {
        if (args_size != 2) return bad_command();
        return run(command_args[1]);
    } else if (strcmp(command_args[0], "init-system") == 0) {
        if (args_size != 1) return bad_command();
        return init();
    } else if (strcmp(command_args[0], "transaction") == 0) {
        if (strcmp(command_args[1], "list-all-transactions") == 0) {
            if (args_size != 2) return bad_command();
            return cli_transaction_list_all_transactions();
        } else if (strcmp(command_args[1], "add-transaction-to-system") == 0) {
            if (args_size != 3) return bad_command();
            return cli_transaction_add_transaction_to_system(command_args[2]);
        } else if (strcmp(command_args[1], "list-utxo") == 0) {
            if (args_size != 2) return bad_command();
            return cli_transaction_list_utxo();
        } else if (strcmp(command_args[1], "get-genesis-private-key") == 0) {
            if (args_size != 2) return bad_command();
            return cli_transaction_get_genesis_transaction_private_key();
        } else {
            return bad_command();
        }
    } else if (strcmp(command_args[0], "cryptography") == 0) {
        if (strcmp(command_args[1], "create-private-key") == 0) {
            if (args_size != 2) return bad_command();
            return cli_cryptography_create_private_key();
        } else if (strcmp(command_args[1], "create-public-key") == 0) {
            if (args_size != 3) return bad_command();
            return cli_cryptography_create_public_key(command_args[2]);
        } else if (strcmp(command_args[1], "sign") == 0) {
            if (args_size != 4) return bad_command();
            return cli_cryptography_sign(command_args[2], command_args[3]);
        } else if (strcmp(command_args[1], "verify") == 0) {
            if (args_size != 5) return bad_command();
            return cli_cryptography_verify(command_args[2], command_args[3], command_args[4]);
        } else {
            return bad_command();
        }
    } else if (strcmp(command_args[0], "block") == 0) {
        if (strcmp(command_args[1], "list-all-blocks") == 0) {
            if (args_size != 2) return bad_command();
            return cli_block_list_all_blocks();
        }else if (strcmp(command_args[1], "get-genesis-hash") == 0) {
            if (args_size != 2) return bad_command();
            return cli_block_get_genesis_block_hash();
         }else if (strcmp(command_args[1], "add-block-to-system")){
            if (args_size != 3) return bad_command();
            return cli_block_add_block_to_system(command_args[2]);
         }else {
            return bad_command();
         }
    } else {
        return bad_command();
    }
}

/**
 * List all the commands.
 * @return If shows the command view successfully, return 0.
 */
int help() {
    char help_string[] =
        "COMMAND			    									DESCRIPTION\n \
help			            								Displays all the commands\n \
quit			            								Exits / terminates the shell with 'Bye'\n \
run script.txt		        								Executes the file SCRIPT.TXT\n \
init-system                 								Initialize the system with the first empty transaction\n \
transaction list-all-transactions       					List all transactions in the current system\n \
transaction add-transaction-to-system transaction.json    	Add the input transaction to the system with the json file\n \
transaction list-utxo                   					List the information of UTXO\n \
transaction get-genesis-private-key							\n \
cryptography create-private-key         					Create a private key string by random\n \
cryptography create-public-key private_key          		Create a public key by the input private key\n \
cryptography sign private_key msg_hash  					Use the private key to sign the hash of the previous outpoint\n \
cryptography verify public_key msg_hash msg_signature   	Verify the encryption by the public key, message, and the signature\n \
block list-all-blocks                                       List all blocks in the current system\n \
block get-genesis-hash                                      Get the hash of the genesis block\n \
block add-block-to-system block.json                        Add the input block to the system with the json file\n";
    printf("%s\n", help_string);
    return 0;
}

/*
 * -----------------------------------------------------------
 * System functions.
 * -----------------------------------------------------------
 */
/**
 * Quit the shell interface.
 * @return Return 0 if exit successfully.
 */
int quit() {
    destroy_block_system();
    destroy_transaction_system();
    destroy_cryptography_system();
    printf("%s\n", "Quit!");
    exit(0);
}

/**
 * Init the cryptography system, transaction system, and the block system.
 * The method will show the genesis transaction's private and the genesis block's hash in the console.
 * @return If run successfully, return 0.
 */
int init() {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_transaction = initialize_transaction_system(false);
    char *genesis_transaction_id = get_transaction_txid(genesis_transaction);
    printf("Genesis transaction id: %s\n", genesis_transaction_id);
    initialize_block_system(false);
    char* genesis_block_hash = get_genesis_block_hash();
    printf("Genesis block hash: %s\n", genesis_block_hash);
    return 0;
}

/**
 * Run the script of a list of commands.
 * @param script The script of a list of commands.
 * @return If run successfully, return 0. If there is a command not defined, return 1. If there is an argument
 * which is a file that not exist, return 2.
 */
int run(char *script) {
    int errCode = 0;
    char line[1000];
    FILE *p = fopen(script, "rt");  // the program is in a file
    if (p == NULL) {
        return bad_command_file_does_not_exist();
    }

    fgets(line, 999, p);
    while (1) {
        errCode = parseInput(line);  // which parses the command and calls interpreter()

        if (feof(p)) break;
        fgets(line, 999, p);
    }
    fclose(p);
    return errCode;
}

/*
 * -----------------------------------------------------------
 * Transaction functions.
 * -----------------------------------------------------------
 */
/**
 * List all the transactions in the system.
 * @return If run successfully, return 0.
 */
int cli_transaction_list_all_transactions() {
//    print_all_transactions();
    return 0;
}

/**
 * According to the arguments in the input script, try to create a new transaction and add it to the system.
 * @param script The input json script should involves the structure like:
 * {
 *     "input_size":1,
 *     "inputs": [
 *         {
 *             "previous_transaction_id" : "64-bit-data",
 *             "output_idx" : 0,
 *             "sender_private_key" : "64-bit-data"
 *         }
 *     ],
 *     "output_size":1,
 *     "outputs":[
 *         {
 *             "receiver_public_key":"64-bit-data",
 *             "amount" : 1
 *         }
 *     ]
 * }
 * @return If run successfully, return 0. If the input script is not found, return 2.
 */
int cli_transaction_add_transaction_to_system(char *script) {
    // Json get parameters: input [{previous_id, output_idx, private_key}], output : [{receiver_public_key, amount}].
    char *buffer;
    long num_bytes;
    FILE *p = fopen(script, "rt");
    if (p == NULL) {
        return bad_command_file_does_not_exist();
    }

    fseek(p, 0L, SEEK_END);
    num_bytes = ftell(p);
    fseek(p, 0L, SEEK_SET);

    buffer = (char *)calloc(num_bytes, sizeof(char));
    if (buffer == NULL) return 1;
    fread(buffer, sizeof(char), num_bytes, p);
    fclose(p);

    double input_size;
    mjson_get_number(buffer, strlen(buffer), "$.input_size", &input_size);
    input_size = (int)input_size;
    transaction_create_shortcut_input *shortcut_input_array =
        (transaction_create_shortcut_input *)malloc(input_size * sizeof(transaction_create_shortcut_input));

    for (int i = 0; i < input_size; i++) {
        char previous_id[100];
        mjson_get_string(buffer, strlen(buffer), "$.inputs[i].previous_transaction_id", previous_id, sizeof(previous_id));
        double output_idx;
        mjson_get_number(buffer, strlen(buffer), "$.inputs[i].output_idx", &output_idx);
        output_idx = (int)output_idx;
        char sender_private_key[100];
        mjson_get_string(buffer, strlen(buffer), "$.inputs[i].sender_private_key", sender_private_key, sizeof(sender_private_key));

        memcpy(shortcut_input_array[i].previous_txid, previous_id, strlen(previous_id));
        shortcut_input_array[i].previous_output_idx = (int)output_idx;
        shortcut_input_array[i].private_key = sender_private_key;
    }

    double output_size;
    mjson_get_number(buffer, strlen(buffer), "$.output_size", &output_size);
    output_size = (int)output_size;
    transaction_create_shortcut_output *shortcut_output_array =
        (transaction_create_shortcut_output *)malloc(sizeof(transaction_create_shortcut_output) * output_size);
    for (int i = 0; i < output_size; i++) {
        char receiver_public_key[100];
        mjson_get_string(buffer, strlen(buffer), "$.outputs[0].receiver_public_key", receiver_public_key, sizeof(receiver_public_key));
        double amount;
        mjson_get_number(buffer, strlen(buffer), "$.outputs[0].amount", &amount);
        amount = (int)amount;
        shortcut_output_array[i].value = (int)amount;
        shortcut_output_array[i].public_key = receiver_public_key;
    }

    free(buffer);

    transaction_create_shortcut *shortcut = (transaction_create_shortcut *)malloc(sizeof(transaction_create_shortcut));
    shortcut->num_of_inputs = input_size;
    shortcut->num_of_outputs = output_size;
    shortcut->inputs = shortcut_input_array;
    shortcut->outputs = shortcut_output_array;

    transaction new_transaction;
    if (create_new_transaction_shortcut(shortcut, &new_transaction)) {
        printf("%s", "create transaction successfully!");
        if (finalize_transaction(&new_transaction)) return 0;
    }
    printf("Input parameters are wrong!\n");
    return 0;
}

/**
 * List the current UTXO data.
 * @return If run successfully, return 0.
 */
int cli_transaction_list_utxo() {
    print_utxo();
    return 0;
}

/**
 * Get the private of the genesis transaction.
 * @return If run successfully, return 0.
 */
int cli_transaction_get_genesis_transaction_private_key() {
    char *temp = get_genesis_transaction_private_key();
    printf("Genesis private key: ");
    print_hex((unsigned char*)temp, strlen(temp));
    return 0;
}

/*
 * -----------------------------------------------------------
 * Cryptography functions
 * -----------------------------------------------------------
 */
/**
 * For cryptography system, create a new private key. The generated private key will be shown in the console.
 * @return If run successfully, return 0.
 */
int cli_cryptography_create_private_key() {
    unsigned char *private_key = get_a_new_private_key();
    printf("Private key: ");
    print_hex((unsigned char*)private_key, strlen((char*)private_key));
    return 0;
}

/**
 * Create a public key for the input private key. The generated public key will be shown in the console.
 * @param private_key Private key.
 * @return If run successfully, return 0.
 */
int cli_cryptography_create_public_key(char *private_key) {
    secp256k1_pubkey *public_key = get_a_new_public_key((char *)private_key);
    public_key_temp = public_key;
    printf("Public key: ");
    print_hex(public_key->data, strlen((char*)public_key->data));
    return 0;
}

/**
 * For cryptography, use the input private key to generate a signature for the message to be sent. The signature will be shown in the console.
 * @param private_key Private key.
 * @param msg_hash Message to be sent.
 * @return If run successfully, return 0.
 */
int cli_cryptography_sign(char *private_key, char *msg_hash) {
    secp256k1_ecdsa_signature *signature = sign((unsigned char*)private_key, (unsigned char*)msg_hash);
    signature_temp = signature;
    printf("Signature: ");
    print_hex(signature->data, strlen((char*)signature->data));
    return 0;
}

/**
 * Verify the cryptography signature with the message hash and public key. If verify successfully, console will
 * notify the user "Result is valid!", otherwise, console will notify the user "Result is invalid!"
 * @param public_key The public key.
 * @param msg_hash The encrypted message.
 * @param signature The signature sent.
 * @return If run successfully, return 0.
 */
int cli_cryptography_verify(char *public_key, char *msg_hash, char *signature) {
    bool result = verify(public_key_temp, (unsigned char*)msg_hash, signature_temp);
    if (result) {
        printf("Result is valid!\n");
    } else {
        printf("Result is invalid!\n");
    }
    return 0;
}

/*
 * -----------------------------------------------------------
 * Block functions
 * -----------------------------------------------------------
 */
/**
 * List all the blocks.
 * @return If runs successfully, return 0.
 */
int cli_block_list_all_blocks() {
    return 0;
}

/**
 * Try to add a block into the system according to the arguments in the input json file.
 * @param script Json file involving the arguments:
 * {
 *     "input_size": 2,
 *     "inputs" : [
 *         "transaction1_id",
 *         "transaction2_id"
 *     ],
 *     "previous_block_hash" : "xxx"
 * }
 * @return If runs successfully, return 0. If the input script file does not exist, return 2.
 */
int cli_block_add_block_to_system(char* script) {
    char *buffer;
    long num_bytes;
    FILE *p = fopen(script, "rt");
    if (p == NULL) {
        return bad_command_file_does_not_exist();
    }

    fseek(p, 0L, SEEK_END);
    num_bytes = ftell(p);
    fseek(p, 0L, SEEK_SET);

    buffer = (char *)calloc(num_bytes, sizeof(char));
    if (buffer == NULL) return 1;
    fread(buffer, sizeof(char), num_bytes, p);
    fclose(p);

    double number_of_transactions;
    mjson_get_number(buffer, strlen(buffer), "$.input_size", &number_of_transactions);

    for(int i=0;i<number_of_transactions;i++){
        char transaction_id[100];
        mjson_get_string(buffer, strlen(buffer), "$.inputs[i]", transaction_id, sizeof(transaction_id));
    }

    for(int i=0; i<number_of_transactions; i++){
        //TODO: append the transactions into the block.
    }

    char previous_block_hash[100];
    mjson_get_string(buffer, strlen(buffer), "$.previous_block_hash", previous_block_hash, sizeof(previous_block_hash));
    block* previous_block =  get_block_by_hash(previous_block_hash);

    block* new_block = create_an_empty_block((int)number_of_transactions);
    if(append_prev_block(previous_block, new_block)){
        if(!finalize_block(new_block)){
            printf("Fail to add the block to the system!");
        }
    }else{
        printf("Blocks cannot be NULL when linking!");
    }
    return 0;
}

/**
 * Get the genesis block's hash. It will be shown in the console.
 * @return If runs successfully, return 0.
 */
int cli_block_get_genesis_block_hash() {
    char *temp = get_genesis_block_hash();
    printf("Genesis block hash: ");
    print_hex((unsigned char*)temp, strlen(temp));
    return 0;
}

/*
 * -----------------------------------------------------------
 * Alert functions
 * -----------------------------------------------------------
 */
/**
 * Alert function for the not defined commands.
 * @return return 1 indicating the input command is not defined.
 */
int bad_command() {
    printf("%s\n", "Unknown Command");
    return 1;
}

/**
 * Alert function for the input file argument in the command not found.
 * @return return 2 indicating the input file argument in the command not found.
 */
int bad_command_file_does_not_exist() {
    printf("%s\n", "Bad command: File not found");
    return 2;
}
