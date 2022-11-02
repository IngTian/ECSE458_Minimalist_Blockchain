#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../model/transaction/transaction.h"
#include "../utils/cryptography.h"
#include "../utils/log_utils.h"
#include "../utils/mjson.h"
#include "shell.h"

secp256k1_pubkey *public_key_temp;
secp256k1_ecdsa_signature *signature_temp;

// system functions
int help();
int quit();
int bad_command();
int bad_command_file_does_not_exist();

// blocktrain functions
// system
int init();
int destroy();
int run(char *script);
//--transactions
int cli_transaction_list_all_transactions();
int cli_transaction_add_transaction_to_system(char *script);
int cli_transaction_list_utxo();
int cli_transaction_get_genesis_transaction_private_key();
//--cryptography
int cli_cryptography_create_private_key();
int cli_cryptography_create_public_key(char *private_key);
int cli_cryptography_sign(char *private_key, char *msg_hash);
int cli_cryptography_verify(char *public_key, char *msg_hash, char *signature);
//--user
int cli_user_get_user_public_key(char *user_id);
int cli_user_get_user_balance(char *user_id);

int interpreter(char *command_args[], int args_size) {
    int i;

    // no argument

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
    } else if (strcmp(command_args[0], "cryptography") == 0) {
        if (strcmp(command_args[1], "get-user-public-key") == 0) {
            if (args_size != 3) return bad_command();
            return cli_user_get_user_public_key(command_args[2]);
        } else if (strcmp(command_args[1], "get-user-balance") == 0) {
            if (args_size != 3) return bad_command();
            return cli_user_get_user_balance(command_args[2]);
        } else {
            return bad_command();
        }
    } else {
        return bad_command();
    }
}

// list all the command
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
user get-user-public-key user_id        					Get the public key of the input user\n \
user get-user-balance user_id           					Get the balance of the user\n";
    printf("%s\n", help_string);
    return 0;
}

// quit the shell interface
int quit() {
    destroy_transaction_system();
    destroy_cryptography_system();
    printf("%s\n", "Quit!");
    exit(0);
}

int init() {
    initialize_cryptography_system(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    transaction *genesis_transaction = initialize_transaction_system();
    char *genesis_transaction_id = get_transaction_txid(genesis_transaction);
    printf("Genesis transaction id: %s\n", genesis_transaction_id);
    return 0;
}

// run SCRIPT.TXT		Executes the file SCRIPT.TXT
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

//-------------------------------Transaction-------------------------

// listAllTransactions     List all transactions in the current system
int cli_transaction_list_all_transactions() {
    print_all_transactions();
    return 0;
}

int cli_transaction_add_transaction_to_system(char *script) {
    // json get parameters: input [{previous_id, output_idx, private_key}], output : [{receiver_public_key, amount}]
    char *buffer;
    long numbytes;
    FILE *p = fopen(script, "rt");  // the program is in a file
    if (p == NULL) {
        return bad_command_file_does_not_exist();
    }

    fseek(p, 0L, SEEK_END);
    numbytes = ftell(p);
    fseek(p, 0L, SEEK_SET);

    buffer = (char *)calloc(numbytes, sizeof(char));
    if (buffer == NULL) return 1;
    fread(buffer, sizeof(char), numbytes, p);
    fclose(p);

    printf("%s\n", buffer);

    //     double val;                                       // Get `a` attribute
    // mjson_get_number(buffer, strlen(buffer), "$.input_size", &val);  // into C variable `val`
    //   printf("a: %g\n", val);

    double input_size;
    mjson_get_number(buffer, strlen(buffer), "$.input_size", &input_size);
    input_size = (int)input_size;
    printf("Input size : %lf\n", input_size);
    transaction_create_shortcut_input *shortcut_input_array =
        (transaction_create_shortcut_input *)malloc(input_size * sizeof(transaction_create_shortcut_input));
    for (int i = 0; i < input_size; i++) {
        const char previous_id[100];
        mjson_get_string(buffer, strlen(buffer), "$.inputs[i].previous_transaction_id", previous_id,
                         sizeof(previous_id));
        double output_idx;
        mjson_get_number(buffer, strlen(buffer), "$.inputs[i].output_idx", &output_idx);
        output_idx = (int)output_idx;
        const char sender_private_key[100];
        mjson_get_string(buffer, strlen(buffer), "$.inputs[i].sender_private_key", sender_private_key,
                         sizeof(sender_private_key));

        memcpy(shortcut_input_array[i].previous_txid, previous_id, strlen(previous_id));
        shortcut_input_array[i].previous_output_idx = output_idx;
        shortcut_input_array[i].private_key = sender_private_key;
    }

    double output_size;
    mjson_get_number(buffer, strlen(buffer), "$.output_size", &output_size);
    output_size = (int)output_size;
    printf("Output size: %lf\n", output_size);

    transaction_create_shortcut_output *shortcut_output_array =
        (transaction_create_shortcut_output *)malloc(sizeof(transaction_create_shortcut_output) * output_size);

    for (int i = 0; i < output_size; i++) {
        const char receiver_public_key[100];
        mjson_get_string(buffer, strlen(buffer), "$.outputs[0].receiver_public_key", receiver_public_key,
                         sizeof(receiver_public_key));
        // secp256k1_pubkey *new_public_key = (secp256k1_pubkey *)malloc(sizeof(secp256k1_pubkey));
        // new_public_key->data = receiver_public_key;

        double amount;
        mjson_get_number(buffer, strlen(buffer), "$.outputs[0].amount", &amount);
        amount = (int)amount;
        shortcut_output_array[i].value = amount;
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

// list-utxo                   List the information of UTXO
int cli_transaction_list_utxo() {
    print_utxo();
    return 0;
}

/**
 * Get the genesis transaction's private key
 */
int cli_transaction_get_genesis_transaction_private_key() {
    char *temp = get_genesis_transaction_private_key();
    printf("Genesis private key: ");
    print_hex(temp, strlen(temp));
    return 0;
}

/**
 * Get the genesis transaction's private key
 */
int cli_transaction_get_genesis_transaction_private_key() {
    char *temp = get_genesis_transaction_private_key();
    printf("Genesis private key: ");
    print_hex(temp, strlen(temp));
    return 0;
}

int cli_cryptography_create_private_key() {
    unsigned char *private_key = get_a_new_private_key();
    printf("Private key: ");
    print_hex(private_key, strlen(private_key));
    return 0;
}

int cli_cryptography_create_public_key(char *private_key) {
    secp256k1_pubkey *public_key = get_a_new_public_key((unsigned char *)private_key);
    public_key_temp = public_key;
    printf("Public key: ");
    print_hex(public_key->data, strlen(public_key->data));
    return 0;
}

int cli_cryptography_sign(char *private_key, char *msg_hash) {
    secp256k1_ecdsa_signature *signature = sign(private_key, msg_hash);
    signature_temp = signature;
    printf("Signature: ");
    print_hex(signature->data, strlen(signature->data));
    return 0;
}

int cli_cryptography_verify(char *public_key, char *msg_hash, char *signature) {
    //	secp256k1_pubkey* pub_key = (secp256k1_pubkey *)malloc(sizeof(secp256k1_pubkey));
    //	secp256k1_ecdsa_signature* sign=(secp256k1_ecdsa_signature *)malloc(sizeof(secp256k1_ecdsa_signature));
    //	pub_key->data=(unsigned char *) public_key;
    //	sign->data=(unsigned char*) signature;
    //	bool result = verify(pub_key, msg_hash, sign);
    //	if(result){
    //		printf("Result is valid!");
    //	}else{
    //		printf("Result is invalid!");
    //	}

    bool result = verify(public_key_temp, msg_hash, signature_temp);
    if (result) {
        printf("Result is valid!\n");
    } else {
        printf("Result is invalid!\n");
    }
    return 0;
}

//-------------------------------User---------------------------------
int cli_user_get_user_public_key(char *user_id) { return 0; }

int cli_user_get_user_balance(char *user_id) { return 0; }



//--------------------------------alert functions--------------------

int bad_command() {
    printf("%s\n", "Unknown Command");
    return 1;
}

int bad_command_file_does_not_exist() {
    printf("%s\n", "Bad command: File not found");
    return 3;
}
