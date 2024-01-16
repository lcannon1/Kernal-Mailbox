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