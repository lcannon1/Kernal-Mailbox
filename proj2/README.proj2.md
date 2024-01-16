# Coding Difficulties
During this project, I faced a lot of difficulties while implementing code for syscalls. Aside from project 0, I had not had any prior experience in coding outside of userspace and, because of this, was not able to determine solutions for all of the problems I faced, regarding user permission, locking, and memory managment. 

# Function Discriptions
create_mbox_421():
Checks variable validity and id availability, initializing and adding a mailbox to the end of the mailbox list, if all checks pass, and returning appropriate errors, if any check fails.

remove_mbox_421:
Checks variable validity and checks requested mailbox for messages, removing the mailbox from the mailbox list, if all checks pass, and returning appropriate errors, if any check fails.

reset_mbox_421():
Goes through each mailbox in the list, removes any present messages and removing the mailbox from the list, before moving to the next.

count_mbox_421():
Goes through each mailbox in the list, adding to a counter for each one and returning the counter to the user.

list_mbox_421():
Checks variable validity and checks requested mailbox for messages, removing the mailbox from the mailbox list, creates a list of all ids of mailboxes in the list and returning the list to the user, if all checks pass, and returning appropriate errors, if any check fails.

send_msg_421():
Checks variable validity and checks requested mailbox for messages, removing the mailbox from the mailbox list. If all checks pass, goes through each mailbox in the list, to find the one with the correct id or returning an error if none are found. If the mailbox is found, initializes a message, copies user message to the initialized message and adds the message to the end of the mailbox's message list.

recv_msg_421():
Goes through each mailbox in the list, to find the one with the correct id or returning an error if none are found. If the mailbox is found, gets the first message in the message list and returns the message to the user, before removing the message from the mailbox. Otherwise returns an appropriate error.

peek_msg_421():
Goes through each mailbox in the list, to find the one with the correct id or returning an error if none are found. If the mailbox is found, gets the first message in the message list and returns the message to the user. Otherwise returns an appropriate error.

count_msg_421():
Goes through each mailbox in the list, to find the one with the correct id or returning an error if none are found. Goes through each message in the mailbox, adding to a counter for each one and returning the counter to the user.

len_msg_421():
Goes through each mailbox in the list, to find the one with the correct id or returning an error if none are found. If the mailbox is found, gets the first message in the message list and returns the message length. Otherwise returns an appropriate error.

print_mbox_421():
Goes through each mailbox in the list, to find the one with the correct id or returning an error if none are found. If the mailbox is found, goes through each message in the mailbox, printing each one by individual byte in hexadecimal form. Otherwise returns an appropriate error. Prints to printk(KERN_INFO).

# Userspace Prototype
I created the userspace prototype because I was unsure of how to make an IPC and thought it would be easier to figure out and tweak in userspace. I implemented all required functions in the userspace, but not permission handeling or locking. The functions I implemented in the userspace version of the code seem to work well, based on my testing of the functions.
