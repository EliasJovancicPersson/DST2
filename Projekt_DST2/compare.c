#include "kernel_functions.h"

int cmp_tcb_priority(const void *a, const void *b) {
    const TCB *tcb1 = (const TCB *)a;
    const TCB *tcb2 = (const TCB *)b;
    
    if (tcb1->Deadline < tcb2->Deadline)
        return -1;
    else if (tcb1->Deadline > tcb2->Deadline)
        return 1;
    else
        return 0;
}
