#include <linux/string.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/semaphore.h>
#include <linux/cred.h>
#include <linux/rwsem.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

// Struct definition for mailbox
typedef struct mail {
    unsigned long id;
    struct list_head message_list;
    struct list_head mailbox_link;
    struct rw_semaphore box_lock;
} mailbox;

// Struct definition for messages
typedef struct msgs {
    char *message;
    struct list_head message_link;
} messages;

// Global semaphore initialization
struct rw_semaphore mbox_lock;
DECLARE_RWSEM(mbox_lock);

// Global mailbox list initialization
LIST_HEAD(mboxes);

SYSCALL_DEFINE1(create_mbox_421, unsigned long, id) {
    mailbox* mbox = (mailbox*)kmalloc(sizeof(mailbox), GFP_KERNEL);
    kuid_t root_id;
    struct list_head *pos;
	struct list_head *tmp;

    // Check for root user
    root_id.val = 0;
    if(!(uid_eq(get_current_cred()->uid, root_id))){
        // Permission denied
        return -EPERM;
    }

    // Check for existing mailbox id
    down_write(&mbox_lock);
	list_for_each_safe (pos, tmp, &mboxes) {
		mailbox* box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);

        // Check current box for requested id
        down_read(&box->box_lock);
		if (box->id == id) {
            up_read(&box->box_lock);
            up_write(&mbox_lock);
            
			// Mailbox already exists
            return -EEXIST;
		}
        up_read(&box->box_lock);

	}

    // Initialize created mailbox
    init_rwsem(&mbox->box_lock);
    mbox->id = id;
    INIT_LIST_HEAD(&mbox->message_list);
    list_add_tail(&mbox->mailbox_link, &mboxes);
    up_write(&mbox_lock);

    return 0;
}

SYSCALL_DEFINE1(remove_mbox_421, unsigned long, id){
    struct list_head* pos;
    struct list_head* tmp;
	struct list_head *pos2;
	struct list_head *tmp2;
    mailbox* box = NULL;
    kuid_t root_id;

    // Check for root user
    root_id.val = 0;
    if(!(uid_eq(get_current_cred()->uid, root_id))){
        // Permission denied
        return -EPERM;
    }

    // Remove mailbox with matching id
    down_write(&mbox_lock);
    list_for_each_safe(pos, tmp, &mboxes) {
        box = list_entry(pos, mailbox, mailbox_link);

        down_read(&box->box_lock);

        // Check current id against requested id
        if (box->id == id) {

            // Check for messages in current mailbox
            list_for_each_safe (pos2, tmp2, &box->message_list) {
                up_read(&box->box_lock);
                up_write(&mbox_lock);

				// Mailbox not empty
                return -ENOTEMPTY;
			}

            // Remove mailbox from list
            up_read(&box->box_lock);
            list_del(pos);
            kfree(box);
            up_write(&mbox_lock);
            return 0;
        }
        up_read(&box->box_lock);
    }
    up_write(&mbox_lock);

    // Mailbox not found
    return -ENOENT;
}

SYSCALL_DEFINE0(reset_mbox_421){
    struct list_head* pos;
    struct list_head* tmp;
    struct list_head *pos2;
	struct list_head *tmp2;
    mailbox* box = NULL;
    messages* msg = NULL;
    kuid_t root_id;

    // Check for root user
    root_id.val = 0;
    if(!(uid_eq(get_current_cred()->uid, root_id))){
        // Permission denied
        return -EPERM;
    }

    // Remove each mailbox in list
    down_write(&mbox_lock);
    list_for_each_safe(pos, tmp, &mboxes) {
        box = list_entry(pos, mailbox, mailbox_link);

        // Check for and remove all messages in mailbox
        down_write(&box->box_lock);
		list_for_each_safe (pos2, tmp2, &box->message_list) {
            msg = list_entry(pos2, messages, message_link);
            list_del(pos2);
            kfree(msg);
		}
        up_write(&box->box_lock);

        // Remove mailbox from list
        list_del(pos);
        kfree(box);
    }
    up_write(&mbox_lock);
    
    return 0;
}

SYSCALL_DEFINE0(count_mbox_421){
    struct list_head* pos;
    struct list_head* tmp;
    long count = 0;

    // Count each mailbox in list
    down_read(&mbox_lock);
    list_for_each_safe(pos, tmp, &mboxes) {
        count++;
    }
    up_read(&mbox_lock);

    return count;
}

SYSCALL_DEFINE2(list_mbox_421, unsigned long __user *, mbxes, long, k){
    struct list_head* pos;
    struct list_head* tmp;
    long count = 0;
    long cpy_byte;
    long* id_list = (long*)kmalloc(sizeof(long)*80, GFP_KERNEL);
    mailbox* box = NULL;

    // Check for negative k
    if(k < 0){
        // Invalid argument
        return -EINVAL;
    }

    // Check user pointer
    cpy_byte = access_ok(mbxes, sizeof(unsigned long*)*k);
    if(cpy_byte == 0){
        // Invalid pointer
        return -EFAULT;
    }

    // Add each id to id_list
    down_read(&mbox_lock);
    list_for_each_safe(pos, tmp, &mboxes) {
        box = list_entry(pos, mailbox, mailbox_link);

        // Add ids until limit is reached
        if(count < k){     
            down_read(&box->box_lock);       
            id_list[count] = box->id;
            count++;
            up_read(&box->box_lock);
        }

    }
    up_read(&mbox_lock);

    mbxes = id_list;
    return count;    
}

SYSCALL_DEFINE3(send_msg_421, unsigned long, id, 
                const unsigned char __user *, msg, long, n){
	struct list_head *pos;
	struct list_head *tmp;
    long cpy_byte;
	mailbox *box = NULL;
    messages* message = (messages*)kmalloc(sizeof(messages), GFP_KERNEL);
    message->message = (char*)kmalloc(n + 1, GFP_KERNEL);

    // Check for null pointer
    if(msg == NULL){
        // Invalid pointer
        return -EFAULT;
    }
    
    // Check for negative n
    if(n < 0){
        // Invalid argument
        return -EINVAL;
    }

    // Check user pointer
    cpy_byte = access_ok(msg, sizeof(unsigned char*)*n);
    if(cpy_byte == 0){
        // Invalid pointer
        return -EFAULT;
    }
    
    // Add user message to requested mailbox
    down_write(&mbox_lock);
	list_for_each_safe (pos, tmp, &mboxes) {
        box = list_entry(pos, mailbox, mailbox_link);

        // Check current id against requested id
        down_write(&box->box_lock);
		if (box->id == id) {

            // Add message to mailbox
            if(n > 0){
                cpy_byte = __copy_from_user(message->message, msg, n);
                if(cpy_byte != 0){
                    up_write(&box->box_lock);
                    up_write(&mbox_lock);

                    // Invalid pointer
                    return -EFAULT;
                }
            }

            list_add_tail(&message->message_link, &box->message_list);
            up_write(&box->box_lock);
            up_write(&mbox_lock);
            return n;			
		}
        up_write(&box->box_lock);

	}
    up_write(&mbox_lock);

    // Mailbox not found
	return -ENONET;     
}

SYSCALL_DEFINE3(recv_msg_421, unsigned long, id, unsigned char __user *, 
                msg, long, n){
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;
    long cpy_byte;
	mailbox *box = NULL;
	struct list_head *pos2;
	struct list_head *tmp2;
    messages* tmp_msg = NULL;
    const char* cpy_msg;

    // Check for null pointer
    if(msg == NULL){
        // Invalid pointer
        return -EFAULT;
    }
    
    // Check for negative n
    if(n < 0){
        // Invalid argument
        return -EINVAL;
    }

    // Check user pointer
    cpy_byte = access_ok(msg, sizeof(unsigned char*)*n);
    if(cpy_byte == 0){
        // Invalid pointer
        return -EFAULT;
    }
    
    // Send message to user and remove message from mailbox
    down_write(&mbox_lock);
	list_for_each_safe (pos, tmp, &mboxes) {
        box = list_entry(pos, mailbox, mailbox_link);

        // Check current id against requested id
        down_write(&box->box_lock);
		if (box->id == id) {

            // Finds first message in mailbox
			list_for_each_safe (pos2, tmp2, &box->message_list) {
                count++;
				if(count == 1){
                    tmp_msg = list_entry(pos2, messages, message_link);

                    // Removes message from mailbox without sending to user
                    if(n == 0){
                        list_del(pos2);
                        kfree(tmp_msg);
                        up_write(&box->box_lock);
                        up_write(&mbox_lock);
                        return 0;
                    }

                    cpy_msg = tmp_msg->message;

                    // Returns up to n bytes of the message
                    if(strlen(cpy_msg) > n){
                        cpy_byte = __copy_to_user(msg, cpy_msg, n);
                        if(cpy_byte != 0){
                            up_write(&box->box_lock);
                            up_write(&mbox_lock);

                            // Invalid pointer
                            return -EFAULT;
                        }
                        list_del(pos2);
                        kfree(tmp_msg);
                        up_write(&box->box_lock);
                        up_write(&mbox_lock);
                        return n;
                    }

                    // Returns the whole message
                    else{
                        n = strlen(cpy_msg);
                        cpy_byte = __copy_to_user(msg, cpy_msg, n);
                        if(cpy_byte != 0){
                            up_write(&box->box_lock);
                            up_write(&mbox_lock);

                            // Invalid pointer
                            return -EFAULT;
                        }
                        list_del(pos2);
                        kfree(tmp_msg);
                        up_write(&box->box_lock);
                        up_write(&mbox_lock);
                        return cpy_byte;
                    }
                }
			}			
		}
        up_write(&box->box_lock);

	}
    up_write(&mbox_lock);

	return count;     
}

SYSCALL_DEFINE3(peek_msg_421, unsigned long, id, unsigned char __user *,
                 msg, long, n){
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;
    long cpy_byte;
	mailbox *box = NULL;
	struct list_head *pos2;
	struct list_head *tmp2;
    messages* tmp_msg = NULL;
    const char* cpy_msg;

    // Check for root user
    if(msg == NULL){
        // Invalid pointer
        return -EFAULT;
    }

    // Check for negative n
    if(n < 0){
        // Invalid argument
        return -EINVAL;
    }

    // Checks user pointer
    cpy_byte = access_ok(msg, sizeof(unsigned char*)*n);
    if(cpy_byte == 0){
        // Invalid pointer
        return -EFAULT;
    }
    
    // Send message to user without removing message from mailbox
    down_read(&mbox_lock);
	list_for_each_safe (pos, tmp, &mboxes) {
        box = list_entry(pos, mailbox, mailbox_link);

        // Check current id against requested id
        down_read(&box->box_lock);
		if (box->id == id) {

            // Finds first message in mailbox
			list_for_each_safe (pos2, tmp2, &box->message_list) {
                count++;
				if(count == 1){
                    tmp_msg = list_entry(pos2, messages, message_link);

                    // Returns nothing to the user
                    if(n == 0){
                        return 0;
                    }

                    cpy_msg = tmp_msg->message;

                    // Returns up to n bytes of the message
                    if(strlen(cpy_msg) > n){
                        cpy_byte = __copy_to_user(msg, cpy_msg, n);
                        if(cpy_byte != 0){
                            up_read(&box->box_lock);
                            up_read(&mbox_lock);

                            // Invalid pointer
                            return -EFAULT;
                        }
                        up_read(&box->box_lock);
                        up_read(&mbox_lock);
                        return n;
                    }

                    // Returns the whole message
                    else{
                        n = strlen(cpy_msg);
                        cpy_byte = __copy_to_user(msg, cpy_msg, n);
                        if(cpy_byte != 0){
                            up_read(&box->box_lock);
                            up_read(&mbox_lock);

                            // Invalid pointer
                            return -EFAULT;
                        }
                        up_read(&box->box_lock);
                        up_read(&mbox_lock);
                        return cpy_byte;
                    }
                }
			}			
		}
        up_read(&box->box_lock);

	}
    up_read(&mbox_lock);

	return count;     
}

SYSCALL_DEFINE1(count_msg_421, unsigned long, id){
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;
	struct list_head *pos2;
	struct list_head *tmp2;

    // Counts all messages in requested mailbox
    down_read(&mbox_lock);
	list_for_each_safe (pos, tmp, &mboxes) {
		mailbox* box = NULL;
        box = list_entry(pos, mailbox, mailbox_link);

        // Checks current id against requested id
        down_read(&box->box_lock);
		if (box->id == id) {

            // Adds to count for each message in mailbox
			list_for_each_safe (pos2, tmp2, &box->message_list) {
				count++;
			}
		}
        up_read(&box->box_lock);

	}
    up_read(&mbox_lock);

	return count;
}

SYSCALL_DEFINE1(len_msg_421, unsigned long, id){
	struct list_head *pos;
	struct list_head *tmp;
	long count = 0;
    long msg_length =  0;
	mailbox* box = NULL;
	struct list_head *pos2;
	struct list_head *tmp2;
    messages* msg = NULL;

    // Returns length of first message in requested mailbox
    down_read(&mbox_lock);
	list_for_each_safe (pos, tmp, &mboxes) {
        box = list_entry(pos, mailbox, mailbox_link);

        // Checks current id against requested id
        down_read(&box->box_lock);
		if (box->id == id) {

            // Finds length of first message in mailbox
			list_for_each_safe (pos2, tmp2, &box->message_list) {

                // Finds only the first message
                count++;
				if(count == 1){

                    // Returns length of first message
                    msg = list_entry(pos2, messages, message_link);
                    msg_length = strlen(msg->message);
                }
			}	
            up_read(&box->box_lock);
            up_read(&mbox_lock);
            return msg_length;		
		}
        up_read(&box->box_lock);

	}
    up_read(&mbox_lock);

    // Mailbox not found
	return -ENONET; 
}

SYSCALL_DEFINE1(print_mbox_421, unsigned long, id){
    struct list_head *pos;
	struct list_head *tmp;
    long count = 1;
    unsigned long i;
	mailbox* box = NULL;
	struct list_head *pos2;
	struct list_head *tmp2;
	messages* msg = NULL;

    // Print all messages in requested mailbox
    down_read(&mbox_lock);
	list_for_each_safe (pos, tmp, &mboxes) {
        box = list_entry(pos, mailbox, mailbox_link);

        // Checks current id against requested id
        down_read(&box->box_lock);
		if (box->id == id) {

            // Prints all messages in mailbox
			list_for_each_safe (pos2, tmp2, &box->message_list) {

                // Used to ensure correct message spacing
                if(count == 1){
                    count++;
                }
                else{
                    printk(KERN_INFO "\n---\n");
                }

                // Prints each message by individual byte
                msg = list_entry(pos2, messages, message_link);
                for(i = 1; i < strlen(msg->message)-1; i++){
                    printk(KERN_INFO "%02x ", msg->message[i-1]);
                    if(i % 16 == 0){
                        printk(KERN_INFO "\n");
                    }
                }
			}

            // Completes print with newline character
            printk(KERN_INFO "\n");
            up_read(&box->box_lock);
            up_read(&mbox_lock);
            return 0;
		}
        up_read(&box->box_lock);

	}
    up_read(&mbox_lock);
    // Mailbox not found
	return -ENONET; 
}