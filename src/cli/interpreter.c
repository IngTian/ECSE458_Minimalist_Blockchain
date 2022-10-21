#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>

#include "../model/transaction/transaction.h"
#include "../utils/cryptography.h"

//system functions
int help();
int quit();
int bad_command();
int bad_command_file_does_not_exist();

//blocktrain functions
//system
int init();
int run(char* script);
//--transactions
int cli_transaction_list_all_transactions();
int cli_transaction_create_new_transaction(char* script);
int cli_transaction_add_transaction_to_system(char* transaction_id);
int cli_list_utxo();
//--cryptography
int cli_cryptography_create_private_key();
int cli_cryptography_create_public_key(char* private_key);
int cli_cryptography_sign(char* private_key, char* msg_hash);
int cli_cryptography_verify(char* public_key, char* msg_hash, char* signature);
//--user
int cli_user_get_user_public_key(char* user_id);
int cli_user_get_user_balance(char* user_id);


int interpreter(char* command_args[], int args_size){

    int i;

    //no argument
	if ( args_size < 1) return badcommand();
    
    // for ( i=0; i<args_size; i++){ //strip spaces new line etc
	// 	command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	// }
    
    if (strcmp(command_args[0], "help")==0){
	    if (args_size != 1) return badcommand();
	    return help();
	} else if (strcmp(command_args[0], "quit")==0) {
		if (args_size != 1) return badcommand();
		return quit();
	}else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand();
		return run(command_args[1]);
    }else if (strcmp(command_args[0], "init-system")==0) {
		if (args_size != 1) return badcommand();
		return init();
    }else if (strcmp(command_args[0], "transaction")==0) {
		if (strcmp(command_args[1], "list-all-transactions")==0) {
			if (args_size != 2) return badcommand();
			return cli_transaction_list_all_transactions();
		}else if (strcmp(command_args[1], "create-new-transaction")==0){
			if (args_size != 3) return badcommand();
			return cli_transaction_create_new_transaction(command_args[2]);
		}else if (strcmp(command_args[1], "add-transaction-to-system")==0){
			if (args_size != 3) return badcommand();
			return cli_transaction_add_transaction_to_system(command_args[2]);
		}else if (strcmp(command_args[1], "list-utxo")==0){
			if (args_size != 2) return badcommand();
			return cli_list_utxo();
		}else{
			return badcommand();
		}
	}else if (strcmp(command_args[0], "cryptography")==0) {
		if (strcmp(command_args[1], "create-private-key")==0){
			if (args_size != 2) return badcommand();
			return cli_cryptography_create_private_key();
		}else if (strcmp(command_args[1], "create-public-key")==0){
			if (args_size != 3) return badcommand();
			return cli_cryptography_create_public_key(command_args[2]);
		}else if (strcmp(command_args[1], "sign")==0){
			if (args_size != 4) return badcommand();
			return cli_cryptography_sign(command_args[2],command_args[3]);
		}else if (strcmp(command_args[1], "verify")==0){
			if (args_size != 5) return badcommand();
			return cli_cryptography_verify(command_args[2],command_args[3],command_args[4]);
		}else {
			return badcommand();
		}
	}else if (strcmp(command_args[0], "cryptography")==0) {
		if (strcmp(command_args[1], "get-user-public-key")==0){
			if (args_size != 3) return badcommand();
			return cli_user_get_user_public_key(command_args[2]);
		}else if (strcmp(command_args[1], "get-user-balance")==0){
			if (args_size != 3) return badcommand();
			return cli_user_get_user_balance(command_args[2]);
		}else{
			return badcommand();
		}
	}else{
        return badcommand();
    }
}


//list all the command
int help(){
	char help_string[] = "COMMAND			    DESCRIPTION\n ";
	printf("%s\n", help_string);

    char line[1000];
    FILE *p = fopen("commandLIst.txt","rt");
	if(p == NULL){
		printf("%s\n", "commandList File not found!");
	    return 3;
	}
    fgets(line,999,p);
	while(1){
        printf("%s\n", line);
		if(feof(p)) break;
		fgets(line,999,p);
	}
    fclose(p);

	return 0;
}

//quit the shell interface
int quit(){
	printf("%s\n", "Quit!");
	exit(0);
}

//TODO
int init(){
    initialize_transaction_system();
    initialize_cryptography_system();
    return 0;
}

// run SCRIPT.TXT		Executes the file SCRIPT.TXT
int run(char* script){
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(script,"rt");  // the program is in a file

	if(p == NULL){
		return badcommandFileDoesNotExist();
	}

	fgets(line,999,p);
	while(1){
		errCode = parseInput(line);	// which parses the command and calls interpreter()
		//TODO
        //memset(line, 0, sizeof(line));
		if(feof(p)) break;
		fgets(line,999,p);
	}

    fclose(p);

	return errCode;
}


//-------------------------------Transaction-------------------------

//TODO
// listAllTransactions     List all transactions in the current system
int cli_transaction_list_all_transactions(){
    get_all_transaction();
    return 0;
}


int cli_transaction_create_new_transaction(char* script){

	return 0;
}

int cli_transaction_add_transaction_to_system(char* transaction_id){
    transaction* transaction = ;
	
    return 0;
}

int cli_list_utxo(){
	return 0;
}

//-----------------------------Cryptography--------------------------

int cli_cryptography_create_private_key(){
	char* private_key = get_a_new_private_key();
	printf("Private key: %s\n", private_key);
	return 0;
}

int cli_cryptography_create_public_key(char* private_key){
	secp256k1_pubkey* public_key = get_a_new_public_key(private_key);
	printf("Public key: %s\n", public_key->data);
	return 0;
}

int cli_cryptography_sign(char* private_key, char* msg_hash){
	secp256k1_ecdsa_signature * signature = sign(private_key, msg_hash);
	printf("Signature: %s\n", signature->data);
	return 0;
}

int cli_cryptography_verify(char* public_key, char* msg_hash, char* signature){
	secp256k1_pubkey* pub_key = (secp256k1_pubkey *)malloc(sizeof(secp256k1_pubkey));
	secp256k1_ecdsa_signature* sign=(secp256k1_ecdsa_signature *)malloc(sizeof(secp256k1_ecdsa_signature));
	pub_key->data = public_key;
	sign -> data = signature;
	bool result = verify(pub_key, msg_hash, sign);
	if(result){
		printf("Result is valid!");
	}else{
		printf("Result is invalid!");
	}
	return 0;
}

//-------------------------------User---------------------------------
int get_user_public_key(char* user_id){
	return 0;
}

int cli_user_get_user_balance(char* user_id){
	return 0;
}

//--------------------------------alert functions--------------------

int bad_command()(){
	printf("%s\n", "Unknown Command");
	return 1;
}

int bad_command_file_does_not_exist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}
