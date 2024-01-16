// Used for function testing

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

/*#include <sys/wait.h>
#include <ctype.h>*/

#ifndef __KERNEl__
 #define __user
#endif

#define __NR_create_mbox_421 436
#define __NR_remove_mbox_421 437
#define __NR_reset_mbox_421 438
#define __NR_count_mbox_421 439
#define __NR_list_mbox_421 440
#define __NR_send_msg_421 441
#define __NR_recv_msg_421 442
#define __NR_peek_msg_421 443
#define __NR_count_msg_421 444
#define __NR_len_msg_421 445
#define __NR_print_mbox_421 446

long create_mbox_syscall(unsigned long id){
    return syscall(__NR_create_mbox_421, id);
}

long remove_mbox_syscall(unsigned long id){
    return syscall(__NR_remove_mbox_421, id);
}

long reset_mbox_syscall(void){
    return syscall(__NR_reset_mbox_421);
}

long count_mbox_syscall(void){
    return syscall(__NR_count_mbox_421);
}

long list_mbox_syscall(unsigned long __user *mbxes, long k){
    return syscall(__NR_list_mbox_421, *mbxes, k);
}

long send_msg_syscall(unsigned long id, const unsigned char __user *msg, long n){
    return syscall(__NR_send_msg_421, id, *msg, n);
}

long recv_msg_syscall(unsigned long id, unsigned char __user *msg, long n){
    return syscall(__NR_recv_msg_421, id, *msg, n);
}

long peek_msg_syscall(unsigned long id, unsigned char __user *msg, long n){
    return syscall(__NR_peek_msg_421, id, *msg, n);
}

long count_msg_syscall(unsigned long id){
    return syscall(__NR_count_msg_421, id);
}

long len_msg_syscall(unsigned long id){
    return syscall(__NR_len_msg_421, id);
}

long print_mbox_syscall(unsigned long id){
    return syscall(__NR_print_mbox_421, id);
}

int main(void){
    long test_id = 1;
    long test;
    long k = 5;

    unsigned long* id_list = (unsigned long*)malloc(sizeof(unsigned long) * 80);
    unsigned char* msg = (unsigned char*)malloc(sizeof(char*) * 80);
    memset(msg, 0, sizeof(msg) * 80);

    reset_mbox_syscall();

    test = count_mbox_syscall();
    printf("Mailboxes: %ld\n", test);
    
    for(long i = 0; i < k; i++){
        create_mbox_syscall(i);
    }
    test = count_mbox_syscall();
    printf("Mailboxes: %ld\n", test);

    remove_mbox_syscall(0);
    test = count_mbox_syscall();
    printf("Mailboxes: %ld\n", test);

    k = test - 1;
    k = list_mbox_syscall(id_list, k);
    printf("Mbox ID: %ld\n", k);
    for(long i = 0; i < 9; i++){
        printf("Mbox ID: %ld\n", id_list[i]);
    }

    memcpy(msg, "Hello", 5);

    send_msg_syscall(test_id, msg, 6);
    memcpy(msg, "BlueberryPancakesAreGood", strlen("BlueberryPancakesAreGood"));
    send_msg_syscall(test_id, msg, 25);

    test = count_msg_syscall(test_id);
    printf("Messages: %ld\n", test);

    print_mbox_syscall(test_id);
    printf("Printed\n");

    test = len_msg_syscall(test_id);
    printf("Msg Length: %ld\n", test);

    strcpy((char*) msg, "Not Right");

    recv_msg_syscall(test_id, msg, 5);
    test = count_msg_syscall(test_id);
    printf("New Messages: %ld\nHello Message: %s\n", test, msg);

    test = len_msg_syscall(test_id);
    printf("Msg Length: %ld\n", test);
    
    peek_msg_syscall(test_id, msg, 5);
    test = count_msg_syscall(test_id);
    printf("New Messages: %ld\nDel Message: %s\n", test, msg);

    test = len_msg_syscall(test_id);
    printf("Msg Length: %ld\n", test);

    recv_msg_syscall(test_id, msg, 5);
    test = count_msg_syscall(test_id);
    printf("New Messages: %ld\nHello Message: %s\n", test, msg);

    test = len_msg_syscall(test_id);
    printf("Msg Length: %ld\n", test);
    
    print_mbox_syscall(test_id);

    return 0;
}