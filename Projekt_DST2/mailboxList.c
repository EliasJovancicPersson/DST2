#include "mailboxList.h"
#include <stdlib.h>

/**
 * Inserts a message at the tail of the mailbox queue.
 */
void mailbox_insert_tail(mailbox *mBox, msg *message) {
    if (!mBox || !message) return;

    message->pNext = NULL;
    if (!mBox->pHead) {
        mBox->pHead = message;
        message->pPrevious = NULL;
    } else {
        mBox->pTail->pNext = message;
        message->pPrevious = mBox->pTail;
    }
    mBox->pTail = message;
    mBox->nMessages++;
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
