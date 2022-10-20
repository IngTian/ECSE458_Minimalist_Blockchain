
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "interpreter.h"

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

int main(int argc, char *argv[]) {
    printf("%s\n", "Block chain minimalist shell");
	help();

    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
	int errorCode = 0;					// zero means no error, default

    for (int i=0; i<MAX_USER_INPUT; i++)
	    userInput[i] = '\0';

    //init shell memory
	
    //TODO:
    //mem_init();

    while(1) {
		//whether at the end of the batch mode file
		if(feof(stdin)){
			//redirect to the terminal
			freopen("/dev/tty","r",stdin);
		}

		printf("%c ",prompt);
		fgets(userInput, MAX_USER_INPUT-1, stdin);

		errorCode = parseInput(userInput);
		if (errorCode == -1) exit(99);	// ignore all other errors
		//memset(userInput, 0, sizeof(userInput));
	}

	return 0;
    
}


// Extract words from the input then call interpreter
int parseInput(char ui[]) {

	char tmp[200];
	char *words[100];							
	int a,b;							
	int w=0; // wordID
	int result=1;

    for(a=0; ui[a]==' ' && a<1000; a++);		// skip white spaces

    while(ui[a] != '\0' && a<1000) {
		for(b=0; ui[a]!='\0' && ui[a]!=' ' && ui[a]!=';' && a<1000; a++, b++){
			tmp[b] = ui[a];						// extract a word
		}
	
		tmp[b] = '\0';

		words[w] = strdup(tmp);

		w++;

		// if current char is ; there are two conditions
		// 1. there is no content after the ; -> this condition should be the unknown commmand
		// 2. there are comments after the ; -> run the command before ; , 
		//    then reset the counter w and char array words. At the next cycle of the while loop, 
		//    command after ; will be read into reset words and w.
		if(ui[a]==';'){
			result = interpreter(words, w);
			//there is nothing after ;
			if(ui[a+1]=='\0') {
				return result;
			}

			w=0;
            
            //TODO:
			//memset(words,'\0',sizeof(words));

			//skip white space
			while(ui[a+1]==' '){
				a++;
			}
		}

		if(ui[a]=='\0') break;
		a++; 
		
	}
	return interpreter(words, w);


}

