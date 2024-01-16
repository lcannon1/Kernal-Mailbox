#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

#ifndef __KERNEl__
 #define __user
#endif

// Create new empty mailbox of ID id 0 on success
long create_mbox_421(unsigned long id);

// Removes mailbox of ID id 0 on success
long remove_mbox_421(unsigned long id);

// Deletes all mailboxes and messages 0 on success
long reset_mbox_421(void);

// Returns number of mailboxes
long count_mbox_421(void);

// Returns list of up to k mailbox IDs in user-space vairable mbxes
// error code on failure
long list_mbox_421(unsigned long __user *mbxes, long k);

// Reads user message msg and adds it to existing mailbox
// Returns number of bytes stored on success error code on failure
long send_msg_421(unsigned long id, const unsigned char __user *msg, long n);

// Copies up to n bytes form next message in mailbox to user-space buffer msg
// remove entire message from mailbox
// Returns number of bytes successfully copied
long recv_msg_421(unsigned long id, unsigned char __user *msg, long n);

// Performs recv_msg_421() without removing message
long peek_msg_421(unsigned long id, unsigned char __user *msg, long n);

// Returns number of messages in mailbox error code on failure
long count_msg_421(unsigned long id);

// Returns length of next message that would be returned by recv_msg_421()
// with the same id value error value if now messages in mailbox
long len_msg_421(unsigned long id);

// Prints contents of mailbox as hexadecimal dump to kernel log using printk()
// Returns 0 on success error code on failure
// Messages separated by a line containing three hyphen characters
long print_mbox_421(unsigned long id);

typedef struct mail {
    unsigned long id;
    struct list_head message_list;
    struct list_head mailbox_link;
} mailbox;

typedef struct msgs {
    char *message;
    struct list_head message_link;
} messages;

LIST_HEAD(mboxes);

long create_mbox_421(unsigned long id) {
    mailbox* mbox = (mailbox*)malloc(sizeof(mailbox));
    mbox->id = id;
    INIT_LIST_HEAD(&mbox->message_list);
    list_add_tail(&mbox->mailbox_link, &mboxes);
    return 0;
}

long remove_mbox_421(unsigned long id){
    if(count_msg_421(id) == 0){

        struct list_head* pos;
        struct list_head* tmp;

        list_for_each_safe(pos, tmp, &mboxes) {
            mailbox* box = NULL;
            box = list_entry(pos, mailbox, mailbox_link);

            if (box->id == id) {
                list_del(pos);
                free(box);
            }
        }
        return 0;
    }
    else{
        // Print error mailbox not empty
        return 0;
    }
}

long reset_mbox_421(void){
    if(count_mbox_421() == 0){
        return 0;
    }
    else{
        struct list_head* pos;
        struct list_head* tmp;
        struct list_head *pos2;
		struct list_head *tmp2;

        list_for_each_safe(pos, tmp, &mboxes) {
            mailbox* box = NULL;
            box = list_entry(pos, mailbox, mailbox_link);
			list_for_each_safe (pos2, tmp2, &box->message_list) {
                messages* msg = NULL;
                msg = list_entry(pos2, messages, message_link);
                list_del(pos2);
                free(msg);
			}

            //box = list_entry(pos, mailbox, mailbox_link);
            list_del(pos);
            free(box);
        }
        return 0;
    }
}

long count_mbox_421(void){

    struct list_head* pos;
    struct list_head* tmp;
    long count = 0;

    list_for_each_safe(pos, tmp, &mboxes) {
        count++;
    }
    return count;
}

long list_mbox_421(unsigned long __user *mbxes, long k){
    ////////////////////////////////////////////////////
    struct list_head* pos;
    struct list_head* tmp;
    long count = 0;

    list_for_each_safe(pos, tmp, &mboxes) {
        mailbox* box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);
        if(count < k){
            mbxes[count] = box->id;
            count++;
        }
    }
    return 0;    
}

long send_msg_421(unsigned long id, const unsigned char __user *msg, long n){
    /////////////////////////////////////////
    if(msg == NULL){
        // error return
        return 0;
    }
    
    if(n < 0){
        // Error return
        return 0;
    }
    
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;

	list_for_each_safe (pos, tmp, &mboxes) {
		mailbox *box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);
		if (box->id == id) {
            messages* message = (messages*)malloc(n);

            if(n > 0){
                message->message = (char*)malloc(sizeof(char*)*n);
                memcpy(message->message, msg, n);
                printf("copied: %s\n", message->message);
            }

            list_add_tail(&message->message_link, &box->message_list);
            return n;			
		}
	}
    // Replace with msg len
	return count;     
}

long recv_msg_421(unsigned long id, unsigned char __user *msg, long n){
    /////////////////////////////////////////
    if(msg == NULL){
        // error return
        return 0;
    }

    if(count_msg_421(id) == 0){
        // error return
        return 0;
    }
    
    
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;

	list_for_each_safe (pos, tmp, &mboxes) {
		mailbox *box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);
		if (box->id == id) {
			struct list_head *pos2;
			struct list_head *tmp2;
			list_for_each_safe (pos2, tmp2, &box->message_list) {
                count++;
				if(count == 1){
                    messages* tmp_msg = NULL;
                    tmp_msg = list_entry(pos2, messages, message_link);
                    if(n == 0){
                        list_del(pos2);
                        free(tmp_msg);
                        return 0;
                    }

                    const char* cpy_msg = tmp_msg->message;
                    if(len_msg_421(id) > n){
                        memcpy(msg, cpy_msg, n);
                        list_del(pos2);
                        free(tmp_msg);
                        return n;
                    }
                    else{
                        n = strlen(cpy_msg);
                        memcpy(msg, cpy_msg, n);
                        list_del(pos2);
                        free(tmp_msg);
                        return len_msg_421(id);
                    }
                }
			}			
		}
	}
	return count;     
}

long peek_msg_421(unsigned long id, unsigned char __user *msg, long n){
    /////////////////////////////////////////
    if(msg == NULL){
        // error return
        return 0;
    }

    if(count_msg_421(id) == 0){
        // error return
        return 0;
    }
    
    
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;

	list_for_each_safe (pos, tmp, &mboxes) {
		mailbox *box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);
		if (box->id == id) {
			struct list_head *pos2;
			struct list_head *tmp2;
			list_for_each_safe (pos2, tmp2, &box->message_list) {
                count++;
				if(count == 1){
                    messages* tmp_msg = NULL;
                    tmp_msg = list_entry(pos2, messages, message_link);
                    if(n == 0){
                        list_del(pos2);
                        free(tmp_msg);
                        return 0;
                    }

                    const char* cpy_msg = tmp_msg->message;
                    if(len_msg_421(id) > n){
                        memcpy(msg, cpy_msg, n);
                        return n;
                    }
                    else{
                        n = strlen(cpy_msg);
                        memcpy(msg, cpy_msg, n);
                        return len_msg_421(id);
                    }
                }
			}			
		}
	}
	return count;     
}

long count_msg_421(unsigned long id){
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;

	list_for_each_safe (pos, tmp, &mboxes) {
		mailbox* box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);
		if (box->id == id) {
			struct list_head *pos2;
			struct list_head *tmp2;
			list_for_each_safe (pos2, tmp2, &box->message_list) {
				count++;
			}
		}
	}
	return count;
}

long len_msg_421(unsigned long id){
    ///////////////////////////////////////////
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;
    long length;

	list_for_each_safe (pos, tmp, &mboxes) {
		mailbox* box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);
		if (box->id == id) {
			struct list_head *pos2;
			struct list_head *tmp2;
			list_for_each_safe (pos2, tmp2, &box->message_list) {
                count++;
				if(count == 1){
                   messages* msg = NULL;
                   msg = list_entry(pos2, messages, message_link);
                   length = strlen(msg->message);
                }
			}			
		}
	}
	return length;
}

long print_mbox_421(unsigned long id){
    ////////////////////////////////////
    if(count_msg_421(id) == 0){
        return 0;
    }

    struct list_head *pos;
	struct list_head *tmp;
    long count = 1;

	list_for_each_safe (pos, tmp, &mboxes) {
		mailbox* box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);
		if (box->id == id) {
			struct list_head *pos2;
			struct list_head *tmp2;
			list_for_each_safe (pos2, tmp2, &box->message_list) {
                if(count == 1){
                    count++;
                }
                else{
                    printf("\n---\n");
                }

				messages* msg = NULL;
                msg = list_entry(pos2, messages, message_link);
                for(unsigned long i = 1; i < strlen(msg->message)-1; i++){
                    printf("%02x ", msg->message[i-1]);
                    if(i % 16 == 0){
                        printf("\n");
                    }
                }
			}
            printf("\n");
            return 0;
		}
	}
    // Error return
    return 0;
}