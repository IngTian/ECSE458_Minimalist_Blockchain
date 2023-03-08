#include "model/transaction/transaction.h"
#include "model/transaction/transaction_persistence.h"
#include "model/block/block.h"
#include "model/block/block_persistence.h"
#include "utils/cryptography.h"
#include "utils/mysql_util.h"
#include "utils/log_utils.h"
#include "utils/sys_utils.h"
#include "utils/constants.h"
#include <ctype.h>


#define LOG_SCOPE "performance test"

char* str_trim(char* str) {
    unsigned long str_len = strlen(str);
    unsigned long front_white_spaces, back_white_spaces;

    // Get number of front white spaces.
    for(front_white_spaces = 0; front_white_spaces < str_len; front_white_spaces++)
        if (!isspace(str[front_white_spaces]))
            break;

    // Get number of back white spaces.
    for(back_white_spaces = 0; str_len - 1 - back_white_spaces >= 0; back_white_spaces++)
        if (!isspace(str[str_len - 1 - back_white_spaces]))
            break;

    char *res = (char*)malloc(str_len - front_white_spaces - back_white_spaces);
    memcpy(res, &str[front_white_spaces], str_len-front_white_spaces-back_white_spaces);
    return res;
}

int main() {

    char* a = "      ss  ";
    a = str_trim(a);
    if (strcmp(a, "ss")==0) printf("111");
    return 0;
}
