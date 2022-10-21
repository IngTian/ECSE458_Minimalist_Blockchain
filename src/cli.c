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

    const char *s1 = "{\n"
        "   \"book\":{\n"
        "      \"title\":\"comp251\",\n"
        "      \"id\":114514,\n"
        "      \"category\":[\n"
        "         \"computer\",\n"
        "         \"algorithm\"\n"
        "      ],\n"
        "      \"isbooked\":false,\n"
        "      \"hex\":\"abcd1234\"\n"
        "   }\n"
        "}"
        ;


    //Get a number from the json
    double value=0;
    if (mjson_get_number(s1, strlen(s1),"$.book.id",&value))  // And print it
        printf("The JSON's id is: %f\n", value);

    //Get a string from the json
    const char buf[100];
    if (mjson_get_string(s1, strlen(s1),"$.book.category[0]",buf,sizeof (buf)))  // And print it
        printf("The JSON's first category is: %s\n", buf);

    //Get a boolean from the json
    bool v;
    if(mjson_get_bool(s1, strlen(s1),"$.book.isbooked",&v));
    printf("The JSON's isbooked is: ");
    printf(v ? "true\n" : "false\n");

    //Get a hex from the json
    char* hex_data;
    if(mjson_get_hex(s1, strlen(s1), "$.book.hex", hex_data, sizeof(hex_data)))
        printf("The JSON's hex data is: ");
        print_hex(hex_data, strlen(hex_data));


        // Print into a statically allocated buffer
    mjson_snprintf(buf, sizeof(buf), "{%Q:%d}", "a", 123);
    printf("%s\n", buf);  // {"a":123}

    // Print into a dynamically allocated string
    char *s2 = mjson_aprintf("{%Q:%g}", "a", 3.1415);
    printf("%s\n", s2);  // {"a":3.1415}
    free(s2);            // Don't forget to free an allocated string;



    //    double val;                                       // Get `a` attribute
//    if (mjson_get_number(s, strlen(s), "$.a", &val))  // into C variable `val`
//        printf("a: %g\n", val);                         // a: 1
//
//    const char *buf;  // Get `b` sub-object
//    int len;          // into C variables `buf,len`
//    if (mjson_find(s, strlen(s), "$.b", &buf, &len))  // And print it
//        printf("%.*s\n", len, buf);                     // [2,false]
//
//    int v;                                           // Extract `false`
//    if (mjson_get_bool(s, strlen(s), "$.b[1]", &v))  // into C variable `v`
//        printf("boolean: %d\n", v);                    // boolean: 0



//    char *buf=hash_sha256("caonimabi");
//    print_hex(buf,32);


    return 0;
}
