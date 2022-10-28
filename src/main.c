

#include "model/block/block.h"
int main() {
    initialize_cryptography_system(true);

    block* genesis_block=initialize_block_system();

    block* b1=create_an_empty_block();
    b1->txn_count=114514;
    finalize_block(b1);

    append_prev_block(genesis_block,b1);

    printf("b1's prev block header hash: %s\n",b1->header->prev_block_header_hash);

//    block* b1= create_an_empty_block();
//
//
//    append_prev_block(genesis_block,b1);
//
//    finalize_block(b1);
//
//    printf("Genesis block header's hash: %s\n", sha256_twice(genesis_block->header));
//    printf("B1 header's previous block header hash: %s\n",b1->header->prev_block_header_hash);
//
//    bool valid= verify_block_chain(b1);
//    printf("%d\n",valid);
    return 0;
}
