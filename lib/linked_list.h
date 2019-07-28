#ifndef INCLUDE_GUARD_LIB_LINKED_LIST_H
#define INCLUDE_GUARD_LIB_LINKED_LIST_H

#include <stddef.h>

typedef struct LinkedList_ {
    struct LinkedList_ *prev;
    struct LinkedList_ *next;
    void *data;
} LinkedList;

#define LINKED_LIST_TAKE(node, type) *(type *)(node->data)

void linked_list_init(LinkedList *node, LinkedList *prev, LinkedList *next, void *data, size_t elem_size_byte);

void linked_list_release(LinkedList *node);

void linked_list_insert_next(LinkedList *node, LinkedList *inserted);

void linked_list_erase(LinkedList *node);

#endif