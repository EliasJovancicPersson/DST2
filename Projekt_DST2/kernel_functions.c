#include "kernel_functions.h"
#include "linkedList.h"
#include "mailboxList.h"
#include "compare.h"
#include<limits.h>
#include <stdlib.h>

/* Global variable definitions */
int Ticks = 0;                   /* global sysTick counter */
int KernelMode = INIT;           /* Kernel mode: INIT or RUNNING */
TCB *PreviousTask = NULL;
TCB *NextTask = NULL;        
listobj *leavingObj = NULL;

list *ReadyList = NULL;
list *WaitingList = NULL;
list *TimerList = NULL;

/* idle task
  - infinite loop created with Deadline UINT_MAX to always be last
*/
void idle_task(void) {
    while (1) {
        //does nothing
    }
}
/* Init kernel
  - Set ticks to 0
  - Create RL,WL and TL
  - Create Idle task
  - Set KernelMode to INIT
  - Return OK
*/
exception init_kernel(void) {
    set_ticks(0);
    
    ReadyList = list_create();
    WaitingList = list_create();
    TimerList = list_create();
    if (ReadyList == NULL || WaitingList == NULL || TimerList == NULL) {
        return FAIL;
    }

    exception status = create_task(idle_task, UINT_MAX);
    if (status != OK) {
        return FAIL;
    }
    KernelMode = INIT;
    return OK;
}

/* Creates a new task:
   - Allocates a TCB and initializes its PC, Deadline, and SP.
   - Inserts the TCB into the ReadyList.
   - If the kernel is already running, disables interrupts and performs a context switch.
*/
exception create_task(void (*task_body)(), uint deadline) {
    /* Allocate memory for a new TCB */
    TCB *new_tcb;
    new_tcb = (TCB *) calloc (1, sizeof(TCB));
    if (new_tcb == NULL) {
        return FAIL;
    }

    /* Initialize the TCB */
    new_tcb->Deadline = deadline;
    new_tcb->PC = task_body;
    new_tcb->SP = &(new_tcb->StackSeg [STACK_SIZE - 9]);
    new_tcb->SPSR = 0x21000000;  // Default processor status register value

  

    /* Initialize Stack */
    new_tcb->StackSeg[STACK_SIZE - 2] = 0x21000000;  // Set xPSR (Thread Mode, Thumb)
    new_tcb->StackSeg [STACK_SIZE - 3] = (unsigned int) task_body;
    
    listobj *node = (listobj *)calloc(1,sizeof(listobj));
    if(node == NULL){
      return FAIL;
    }
    node->pNext = node->pPrevious = NULL;
    node->pTask = new_tcb;
    node->nTCnt = 0;
    
    /* Insert into ReadyList */
    if (KernelMode == INIT) {
        if (!list_insert_sort(ReadyList, node, cmp_tcb_priority)) {
            free(new_tcb);
            return FAIL;
        }
        return OK;
    } else {
        isr_off();
        PreviousTask = NextTask; // Save the current task before switching
        if (!list_insert_sort(ReadyList, node, cmp_tcb_priority)) {
            isr_on();
            free(new_tcb);
            return FAIL;
        }
        
        if(NextTask == NULL || new_tcb->Deadline < NextTask->Deadline){//if deadlines true we can assume newTCB is infact first in readylist otherwise the nexttask wont need to change
          NextTask = ReadyList->pHead->pTask;
          SwitchContext();
        }
        isr_on();
        return OK;
    }
}

void run(void) {
  set_ticks(0);
  KernelMode = RUNNING;
  NextTask = ReadyList->pHead->pTask;
  LoadContext_In_Run();
}

/* Terminates the currently running task:
   - Disables interrupts.
   - Frees the TCB of the current task.
   - Extracts the next task from the ReadyList.
   - Switches to the next task's stack and loads its context.
*/
void terminate(void) {
    isr_off();

    if (!ReadyList->pHead) {
      isr_on();  
      return;
    }

    leavingObj = list_remove_head(ReadyList);
    NextTask = ReadyList->pHead->pTask;

    switch_to_stack_of_next_task();
    free(leavingObj->pTask);
    free(leavingObj);

    LoadContext_In_Terminate();
}

mailbox* create_mailbox(uint nMessages, uint nDataSize) {
    mailbox *mBox = malloc(sizeof(mailbox));
    if (!mBox) return NULL;
    mBox->pHead = NULL;
    mBox->pTail = NULL;
    mBox->nDataSize = nDataSize;
    mBox->nMaxMessages = nMessages;
    mBox->nMessages = 0;
    mBox->nBlockedMsg = 0;
    return mBox;
}

exception remove_mailbox(mailbox* mBox) {
    if (!mBox) return FAIL;
    if (mBox->nMessages == 0) {
        free(mBox);
        return OK;
    }
    return NOT_EMPTY;
}

exception send_wait(mailbox* mBox, void* pData) {
    isr_off();

    // If a receiver is waiting, deliver the message immediately
    if (mBox->pHead) {
        memcpy(mBox->pHead->pData, pData, mBox->nDataSize);
        msg *receivedMsg = mailbox_remove_head(mBox);
        mBox->nBlockedMsg--;
        free(receivedMsg);

        // Move the receiving task to the ReadyList
        PreviousTask = NextTask;
        listobj *WaitHead = list_remove_head(WaitingList);
        list_insert_sort(ReadyList, WaitHead,cmp_tcb_priority);
        NextTask = ReadyList->pHead->pTask;
    }else{
        // No receiver -> Block sender
      msg* newMsg = (msg*)malloc(sizeof(msg));
      if (!newMsg) {
          return FAIL;
      }
  
      newMsg->pData = malloc(mBox->nDataSize);
      if (!newMsg->pData) {
          free(newMsg);
          return FAIL;
      }
      newMsg->Status = SENDER;
  
      newMsg->pBlock = ReadyList->pHead;
      
      memcpy(newMsg->pData, pData, mBox->nDataSize);
      
      mailbox_insert_tail(mBox, newMsg);
      mBox->nBlockedMsg++;
  
      // Move sender task to `WaitingList` (Blocking it)
      PreviousTask = NextTask;
      listobj *node = list_remove_head(ReadyList);
      list_insert_sort(WaitingList, node, cmp_tcb_priority);
  
      NextTask = ReadyList->pHead->pTask;
    }

    SwitchContext(); //for some reason we get tb5 in both lists her

    // If the task remains blocked until its deadline is reached
    if (PreviousTask->Deadline <= Ticks) {
        isr_off();
        msg *expiredMsg = mailbox_remove_head(mBox);
        free(expiredMsg);
        isr_on();
        return DEADLINE_REACHED;
    }else{
        return OK;
    }
}



exception receive_wait(mailbox* mBox, void* pData) {
    isr_off();

    if (mBox->pHead) {
        msg *receivedMsg = mailbox_remove_head(mBox);
        if (!receivedMsg) {
            isr_on();
            return FAIL;
        }

        memcpy(pData, receivedMsg->pData, mBox->nDataSize);

        if(mBox->nBlockedMsg > 0) {
            PreviousTask = NextTask;
            list_insert_sort(ReadyList, receivedMsg->pBlock, cmp_tcb_priority);
            NextTask = ReadyList->pHead->pTask;
            mBox->nBlockedMsg--;
        } else {
            free(receivedMsg->pData);
            free(receivedMsg);
        }
    }else{
        msg* newMsg = (msg*)malloc(sizeof(msg));
        if (!newMsg) {
            isr_on();
            return FAIL;
        }

        newMsg->pData = malloc(mBox->nDataSize);
        if (!newMsg->pData) {
            free(newMsg);
            isr_on();
            return FAIL;
        }
        
        newMsg->Status = RECEIVER;
        newMsg->pBlock = ReadyList->pHead;

        mailbox_insert_tail(mBox, newMsg);
        PreviousTask = NextTask;
        
        list_insert_sort(WaitingList, list_remove_head(ReadyList), cmp_tcb_priority);

        NextTask = ReadyList->pHead->pTask;
    }
    SwitchContext();

    // Check if deadline is reached
    if (PreviousTask->Deadline <= Ticks) {
        isr_off();
        msg *expiredMsg = mailbox_remove_head(mBox);
        if (expiredMsg) {
            free(expiredMsg);
        }
        isr_on();
        return DEADLINE_REACHED;
    }
    return OK;
}

exception send_no_wait(mailbox *mBox, void *pData) {
    isr_off();

    // If the mailbox is full, remove the oldest message
    if (mBox->nMessages == mBox->nMaxMessages) {
        msg *oldMsg = mailbox_remove_head(mBox);
        free(oldMsg->pData);
        free(oldMsg);
    }

    msg* newMsg = malloc(sizeof(msg));
    if (!newMsg) {
        isr_on();
        return FAIL;
    }

    newMsg->pData = malloc(mBox->nDataSize);
    if (!newMsg->pData) {
        free(newMsg);
        isr_on();
        return FAIL;
    }

    memcpy(newMsg->pData, pData, mBox->nDataSize);

    // Add new message
    mailbox_insert_tail(mBox, newMsg);

    isr_on();
    return OK;
}


int receive_no_wait(mailbox* mBox, void* pData) {
    isr_off();
    if (!mBox->pHead || mBox->nMessages == 0) {
        isr_on();
        return FAIL;
    }

    msg* oldMsg = mailbox_remove_head(mBox);
    if (!oldMsg) {
        isr_on();
        return FAIL;
    }

    memcpy(pData, oldMsg->pData, mBox->nDataSize);
    free(oldMsg->pData);
    free(oldMsg);

    isr_on();
    return OK;
}

exception wait(uint nTicks) {
    isr_off();
    PreviousTask = NextTask;
    listobj* node = list_remove_head(ReadyList);
    node->nTCnt = nTicks + ticks();
    list_insert_sort(TimerList, node,cmp_tcb_priority);
    
    NextTask = ReadyList->pHead->pTask;
    
    SwitchContext();
    if (PreviousTask->Deadline <= ticks()) {
        return DEADLINE_REACHED;
    }
    return OK;
}

void set_ticks(uint nTicks) {
    Ticks = nTicks;
}

uint ticks(void) {
    return Ticks;
}

uint deadline(void) {
    return NextTask->Deadline;
}

void set_deadline(uint deadline) {
    isr_off();
    NextTask->Deadline = deadline;
    PreviousTask = NextTask;
    list_remove_head(ReadyList);
    list_insert_sort(ReadyList, ReadyList->pHead, cmp_tcb_priority);
    NextTask = ReadyList->pHead->pTask;
    SwitchContext();
    isr_on();
}

void TimerInt(void) {
    Ticks++;
    if (Ticks == 4000) {
        asm("nop");
    }
    
    // Iterate through TimerList to find eligible nodes.
    listobj *node = TimerList->pHead;
    while (node != NULL) {
        listobj *next = node->pNext;  // Save next pointer before unlinking.
        if (node->pTask->Deadline <= Ticks || node->nTCnt <= Ticks) {
            // Unlink the node from TimerList without freeing it.
            node = list_unlink_node(TimerList, node);
            // Insert the node into ReadyList (using sorted insertion if desired).
            list_insert_sort(ReadyList, node, cmp_tcb_priority);
            PreviousTask = NextTask;
            NextTask = node->pTask;
        }
        node = next;
    }
    
    // Process WaitingList (which is sorted by deadline, so only the head is checked).
    while (WaitingList->pHead != NULL &&
           WaitingList->pHead->pTask->Deadline <= Ticks) {
        listobj *wnode = list_remove_head(WaitingList);
        list_insert_sort(ReadyList, wnode, cmp_tcb_priority);
        PreviousTask = NextTask;
        NextTask = wnode->pTask;
    }
}

