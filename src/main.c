

#include "model/block/block.h"
int main() {
    initialize_cryptography_system(true);
    block* genesis_block=initialize_block_system();
    block* b1=create_an_empty_block();
    b1->header->version=114514;
    finalize_block(b1);
    append_prev_block(genesis_block,b1);

    transaction* t;
    t->version=999;

    append_transaction_into_block(b1,t,0);

    bool valid= verify_block_chain(b1);
    return 0;
}
