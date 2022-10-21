#include "cli.h"

#include <assert.h>
#include <glib.h>
#include <secp256k1.h>
#include <stdbool.h>
#include <stdio.h>
#include "utils/mjson.h"
#include "random.h"
#include "utils/cryptography.h"
#include "model/transaction/transaction.h"

int main(int argc, char *argv[]) {
    const char *s = "{\"a\":1,\"b\":[2,false]}";  // {"a":1,"b":[2,false]}

    double val;                                       // Get `a` attribute
    if (mjson_get_number(s, strlen(s), "$.a", &val))  // into C variable `val`
        printf("a: %g\n", val);                         // a: 1

    const char *buf;  // Get `b` sub-object
    int len;          // into C variables `buf,len`
    if (mjson_find(s, strlen(s), "$.b", &buf, &len))  // And print it
        printf("%.*s\n", len, buf);                     // [2,false]

    int v;                                           // Extract `false`
    if (mjson_get_bool(s, strlen(s), "$.b[1]", &v))  // into C variable `v`
        printf("boolean: %d\n", v);                    // boolean: 0



//    char *buf=hash_sha256("caonimabi");
//    print_hex(buf,32);


    return 0;
}
