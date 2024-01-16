// Used for function testing

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include "user_space.h"

int main(void){
    long test_id = 123;
    long test;
    long k = 1;

    unsigned long* id_list = (unsigned long*)malloc(sizeof(unsigned long) * k);
    unsigned char* msg = (unsigned char*)malloc(sizeof(char*) * 80);
    memset(msg, 0, sizeof(msg) * 80);

    test = count_mbox_421();
    create_mbox_421(test_id);
    create_mbox_421(430);
    create_mbox_421(530);
    test = count_mbox_421();
    printf("Mailboxes: %ld\n", test);
    k = test - 1;
    list_mbox_421(id_list, k);
    for(long i = 0; i < k; i++){
        printf("Mbox ID: %ld\n", id_list[i]);
    }

    memcpy(msg, "Hello", 5);
    send_msg_421(test_id, msg, 5);
    memset(msg, 0, sizeof(msg) * 80);

    memcpy(msg, "BlueberryPancakesAreGood", strlen("BlueberryPancakesAreGood"));
    send_msg_421(test_id, msg, strlen((char*) msg));
    memset(msg, 0, sizeof(msg) * 80);

    test = len_msg_421(test_id);
    printf("Msg Length: %ld\n", test);

    print_mbox_421(test_id);

    test = count_msg_421(test_id);
    printf("Messages: %ld\n", test);
    recv_msg_421(test_id, msg, 3);
    printf("New Messages: %ld\nHello Message: %s\n", test, msg);

    test = count_msg_421(test_id);
    peek_msg_421(test_id, msg, 100);
    test = count_msg_421(test_id);
    printf("New Messages: %ld\nDel Message: %s\n", test, msg);
    
    print_mbox_421(test_id);

    return 0;
}