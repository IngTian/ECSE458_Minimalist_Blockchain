#ifndef MINIMALIST_BLOCKCHAIN_SYSTEM_CLI_INTERPRETER_H
#define MINIMALIST_BLOCKCHAIN_SYSTEM_CLI_INTERPRETER_H

#ifdef __cplusplus
extern "C" {
#endif

int interpreter(char* command_args[], int args_size);
int help();

#ifdef __cplusplus
}
#endif

#endif