#include "linkedList.h"
#include <stdlib.h>

list *list_create(void) {
    list *lst = (list *) malloc(sizeof(list));
    if (!lst)
        return NULL;
    lst->pHead = lst->pTail = NULL;
    return lst;
}

void list_destroy(list *lst) {
    if (!lst)
        return;
    listobj *node = lst->pHead;
    while (node) {
        listobj *next = node->pNext;
        free(node);
        node = next;
    }
    free(lst);
}

int list_insert_sort(list *lst, listobj *node, int (*cmp)(const void *, const void *)) {
    if (!lst || !cmp)
        return 0;

    if (lst->pHead == NULL && lst->pTail == NULL) {
        lst->pHead = lst->pTail = node;
        return 1;
    }

    listobj *current = lst->pHead;
    
    while (current != NULL && cmp(node->pTask, current->pTask) > 0) {
        current = current->pNext;
    }
    
    if (current == lst->pHead) {
        node->pNext = lst->pHead;
        lst->pHead->pPrevious = node;
        lst->pHead = node;
    } 
    else if (current == NULL) {
        node->pPrevious = lst->pTail;
        lst->pTail->pNext = node;
        lst->pTail = node;
    } 
    else {
        node->pPrevious = current->pPrevious;
        node->pNext = current;
        if (current->pPrevious) {
            current->pPrevious->pNext = node;
        } else {
            lst->pHead = node;
        }
        current->pPrevious = node;
    }

    return 1;
}

int list_insert_tail(list *lst, void *data) {
    if (!lst) return 0;
    
    listobj *node = (listobj *)malloc(sizeof(listobj));
    if (!node) return 0;

    node->pTask = (TCB *)data;
    node->pMessage = (msg *)data;
    node->pNext = NULL;
    node->pPrevious = lst->pTail;

    if (lst->pTail)
        lst->pTail->pNext = node;
    else
        lst->pHead = node;

    lst->pTail = node;

    return 1;
}

int list_insert_head(list *lst, listobj *data) {
    if (!lst)
        return 0;
    
    // Allocate memory for a new node.
    listobj *node = malloc(sizeof(listobj));
    if (!node)
        return 0;

    // Copy the contents of *data into the newly allocated node.
    *node = *data;  
    // Initialize the pointers of the new node.
    node->pPrevious = NULL;
    node->pNext = lst->pHead;
    
    // Link the old head's previous pointer to the new node.
    if (lst->pHead)
        lst->pHead->pPrevious = node;
    else
        // If the list was empty, new node is also the tail.
        lst->pTail = node;

    // Set the new node as the head of the list.
    lst->pHead = node;

    return 1;
}

// Removes a node from the list without freeing its memory.
listobj *list_unlink_node(list *lst, listobj *node) {
    if (!lst || !node)
        return NULL;
    
    if (node->pPrevious)
        node->pPrevious->pNext = node->pNext;
    else
        lst->pHead = node->pNext;

    if (node->pNext)
        node->pNext->pPrevious = node->pPrevious;
    else
        lst->pTail = node->pPrevious;

    // Clear the node's pointers so it can be safely reinserted.
    node->pNext = node->pPrevious = NULL;
    return node;
}


listobj *list_remove_head(list *lst) {
  
   listobj *node = lst->pHead;
   
    if (!lst || lst->pHead == NULL) {
        return NULL;
    }
    
    if(lst->pHead == lst->pTail){
      lst->pHead = NULL;
      lst->pTail = NULL;
    } else{
      lst->pHead = node->pNext;
      lst->pHead->pPrevious = NULL;
      node->pNext = NULL;
    }
    

   
//    listobj *nextHead = lst->pHead->pNext;
//    if(nextHead = NULL){
//      lst->pHead = NULL;
//      lst->pTail = NULL;
//    } if (lst->pHead != NULL) {
//        lst->pHead->pPrevious = NULL;
//        lst->pHead->pNext = NULL;
//        lst->pHead = nextHead;
//        if(lst->pHead != NULL){
//          lst->pHead->pPrevious = NULL;
//        }else{
//          lst->pTail = NULL;
//        }
//    } 
    return node;
}

void *list_remove_node(list *lst, listobj *node) {
    if (!lst || !node)
        return NULL;

    if (node->pPrevious)
        node->pPrevious->pNext = node->pNext;
    else
        lst->pHead = node->pNext;

    if (node->pNext) {
        node->pNext->pPrevious = node->pPrevious;
    } else {
        lst->pTail = node->pPrevious;
    }

    void *data = (node->pTask) ? (void *)node->pTask : (void *)node->pMessage;
    free(node);
    return data;
}

listobj *list_find(list *lst, const void *key, int (*cmp)(const void *, const void *)) {
    if (!lst || !cmp)
        return NULL;

    listobj *node = lst->pHead;
    while (node) {
        if (cmp(node->pTask, key) == 0)
            return node;
        node = node->pNext;
    }
    return NULL;
}

listobj *list_first(list *lst) {
    return (lst) ? lst->pHead : NULL;
}

listobj *list_next(listobj *node) {
    return (node) ? node->pNext : NULL;
}
