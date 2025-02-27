#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "kernel_functions.h" 
#include <stddef.h>

/* 
   Function prototypes for linked list operations.
   The types "list" and "listobj" are already defined in kernel_functions.h.
*/

/* Creates and returns a new list.
   (A list has only pHead and pTail as defined in kernel_functions.h.)
*/
list *list_create(void);

/* Destroys the list and frees its nodes.
   (It does not free the TCB or msg pointers stored in pTask or pMessage.)
*/
void list_destroy(list *lst);

/* Inserts a new TCB or msg pointer (passed as void* data) into the list in sorted order.
   The comparison function cmp compares two TCB or msg pointers.
   Returns 1 on success, 0 on failure.
*/
int list_insert_sort(list *lst, listobj *node, int (*cmp)(const void *, const void *));

/* Inserts a new TCB or msg pointer at the tail of the list.
   Returns 1 on success.
*/
int list_insert_tail(list *lst, void *data);

/* Inserts a new TCB or msg pointer at the head of the list.
   Returns 1 on success.
*/
int list_insert_head(list *lst, listobj *data);

/* Removes the head node from the list and returns its stored TCB or msg pointer.
   (Used by run() to extract the next task or message.)
*/
listobj *list_remove_head(list *lst);

/* Removes a given node from the list and returns its stored TCB or msg pointer.*/
void *list_remove_node(list *lst, listobj *node);

/* Finds a node in the list that matches the key using cmp.
   Returns a pointer to the list node if found, otherwise NULL.
*/
listobj *list_find(list *lst, const void *key, int (*cmp)(const void *, const void *));

/* Returns the first node in the list.*/
listobj *list_first(list *lst);

/* Returns the next node after the given node.*/
listobj *list_next(listobj *node);

listobj *list_unlink_node(list *lst, listobj *node);

#endif /* LINKEDLIST_H */
