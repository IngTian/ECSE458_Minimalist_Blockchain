#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>


//system functions
int help();
int quit();
int badcommand();
int badcommandFileDoesNotExist();

//blocktrain functions



int interpreter(char* command_args[], int args_size){

    int i;

    //no argument
	if ( args_size < 1) return badcommand();
	
    
    // for ( i=0; i<args_size; i++){ //strip spaces new line etc
	// 	command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	// }
    
    if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommand();
	    return help();
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) return badcommand();
		return quit();
	}else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand();
		return run(command_args[1]);
    else{
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








int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}


int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}