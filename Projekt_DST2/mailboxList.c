#include "mailboxList.h"
#include <stdlib.h>

/**
 * Inserts a message at the tail of the mailbox queue.
 */
void mailbox_insert_tail(mailbox *mBox, msg *message) {
    if (!mBox || !message) return;  // Prevent NULL pointer errors

    // Ensure message's pointers are properly initialized
    message->pNext = NULL;
    message->pPrevious = NULL;

    // If the mailbox is empty, set both head and tail to the new message
    if (!mBox->pHead) {
        mBox->pHead = message;
        mBox->pTail = message;
    } else {
        // Append message to tail
        if (mBox->pTail) {
            mBox->pTail->pNext = message;
        }
        message->pPrevious = mBox->pTail;
        mBox->pTail = message;
    }

    mBox->nMessages++;  // Increment count only when a message is successfully added
}


/**
 * Removes a message from the head of the mailbox queue.
 * Returns the removed message.
 */
msg *mailbox_remove_head(mailbox *mBox) {
    if (!mBox || !mBox->pHead) return NULL;

    msg *message = mBox->pHead;
    mBox->pHead = message->pNext;

    if (mBox->pHead) {
        mBox->pHead->pPrevious = NULL;
    } else {
        mBox->pTail = NULL;
    }

    mBox->nMessages--;
    return message;
}
