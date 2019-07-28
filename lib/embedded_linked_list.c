#include "embedded_linked_list.h"

#include <assert.h>
#include <stddef.h>

void linked_list_init(LinkedList *node, LinkedList *prev, LinkedList *next) {
    if (prev) prev->next = node;
    if (next) next->prev = node;
    node->next = next;
    node->prev = prev;
}

void linked_list_insert_next(LinkedList *node, LinkedList *inserted) {
    assert(node);
    assert(inserted->prev == NULL);
    assert(inserted->next == NULL);
    LinkedList *c = node->next;
    node->next = inserted;
    inserted->prev = node;
    inserted->next = c;
    if (c) c->prev = inserted;
}

void linked_list_erase(LinkedList *node) {
    LinkedList *p = node->prev;
    LinkedList *n = node->next;
    if (p) p->next = n;
    if (n) n->prev = p;
    node->prev = NULL;
    node->next = NULL;
}
