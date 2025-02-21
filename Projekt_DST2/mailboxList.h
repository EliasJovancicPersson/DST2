#ifndef MAILBOXLIST_H
#define MAILBOXLIST_H

#include "kernel_functions.h"  // Use the existing mailbox structure

// Function prototypes
void mailbox_insert_tail(mailbox *mBox, msg *message);
msg *mailbox_remove_head(mailbox *mBox);

#endif
