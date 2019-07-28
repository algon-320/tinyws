#ifndef INCLUDE_GUARD_EMBEDDED_LINKED_LIST_H
#define INCLUDE_GUARD_EMBEDDED_LINKED_LIST_H

typedef struct LinkedList_ {
    struct LinkedList_ *next;
    struct LinkedList_ *prev;
} LinkedList;

void linked_list_init(LinkedList *node, LinkedList *prev, LinkedList *next);
void linked_list_insert_next(LinkedList *node, LinkedList *inserted);
void linked_list_erase(LinkedList *node);

#endif