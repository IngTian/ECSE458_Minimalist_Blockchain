#include <glib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "block.h"
#include "utils/constants.h"
#include "utils/cryptography.h"
#include "../transaction/transaction.h"
#include <sys/time.h>
#include <string.h>
#include <time.h>

static GHashTable  *g_global_block_table;
char *g_genesis_block_hash;


long long get_timestamp(void)
{
    long long tmp;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    tmp = tv.tv_sec;
    tmp = tmp * 1000;
    tmp = tmp + (tv.tv_usec / 1000);

    return tmp;
}


block *initialize_block_system(){
    g_global_block_table= g_hash_table_new(g_str_hash, g_str_equal);
    block* genesis_block= malloc(sizeof(block));
    return genesis_block;
}

block *create_an_empty_block(){
    block* block_create= malloc(sizeof(block));
    block_create->timestamp_create=get_timestamp();
    return block_create;
}

void destroy_block(block* block_destroy){
    for(int i=0;i<block_destroy->transaction_count;i++){
        destroy_transaction(block_destroy->transaction_list[i]);
    }
    free(block_destroy->prev_block_header);
    free(block_destroy->current_block_header);
    free(block_destroy);
}

bool append_prev_block(block* prev_block, block* cur_block){
    if(prev_block==NULL||prev_block->current_block_header==NULL||cur_block==NULL){
        return false;
    }
    cur_block->prev_block_header= malloc(sizeof(block_header));
    cur_block->prev_block_header=prev_block->current_block_header;

    return true;
}

bool finalize_block(block* block_finalize){
    block_finalize->current_block_header= malloc(sizeof(block_header));
    char * current_block_hash;
    if(block_finalize->prev_block_header==NULL){
        current_block_hash="";
    }else{
        current_block_hash=block_finalize->prev_block_header->hash;
    }

    char * time_stamp;
    sprintf(time_stamp, "%lld", block_finalize->timestamp_create);

    strcat(current_block_hash,time_stamp);
    strcat(current_block_hash, hash_struct(block_finalize->transaction_list,sizeof(block_finalize->transaction_list)));

    block_finalize->current_block_header->hash=current_block_hash;

    g_hash_table_insert(g_global_block_table,current_block_hash,block_finalize);

    return true;
}

block *get_block_by_hash(char * hash){
    block* block1= malloc(sizeof(block1));
    block1= g_hash_table_lookup(g_global_block_table,hash);
    return block1;
}

bool append_transaction_into_block(transaction* t, block* block1){
    block1->transaction_count++;
    block1->transaction_list[block1->transaction_count]=t;
    return true;
}









