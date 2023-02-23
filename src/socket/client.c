// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../model/block/block.h"
#include "../model/transaction/transaction.h"
#define PORT 8080
struct UsrData {
    char usr_id[16];
    char usr_pwd[16];
    char usr_nickname[16];
};

struct Person {
    int pid;
    struct UsrData usrData;
};


int main(int argc, char const *argv[]) {
    //    block_header_shortcut block_header={
    //        .prev_block_header_hash=NULL,
    //        .version=123,
    //        .nonce=0,
    //        .nBits=0,
    //        .merkle_root_hash=NULL,
    //        .time=114514
    //    };
    //
    //    //create a new block
    //    block_create_shortcut block_data={
    //        .header=&block_header,
    //        .transaction_list=NULL
    //    };
    //
    //    block* block1= (block *)malloc(sizeof(block));
    //    create_new_block_shortcut(&block_data,block1);

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    char *hello = "Hello, this is message from client!!!!";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }



//    transaction transaction1;
//    transaction1.version=114514;
//    transaction1.lock_time=2023;
//    transaction1.tx_out_count=1;
//    transaction1.tx_in_count=1;
//    transaction_output output;
//    output.value=99;
//    memcpy(transaction1.tx_outs,(char *)&output, sizeof(output));
    struct Person person;
    struct UsrData sendUser;
    person.pid=99;
    memcpy(sendUser.usr_id, "100001", sizeof("100001"));
    memcpy(sendUser.usr_pwd, "123456", sizeof("123456"));
    memcpy(sendUser.usr_nickname, "Joe", sizeof("Joe"));
    person.usrData=sendUser;

    send(sock, (char *)&person, sizeof(person), 0);
    printf("Client: person sent\n");
//    valread = read(sock, buffer, 1024);
    recv(sock, buffer, 1024, 0);
    printf("%s\n", buffer);

    // closing the connected socket
    close(client_fd);
    return 0;
}
