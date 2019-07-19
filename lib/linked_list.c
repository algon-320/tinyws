#include "linked_list.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void linked_list_init(LinkedList *node, LinkedList *prev, LinkedList *next, void *data, size_t elem_size_byte) {
    if (prev) prev->next = node;
    if (next) next->prev = node;
    node->prev = prev;
    node->next = next;
    node->data = malloc(elem_size_byte);
    if (data) memcpy(node->data, data, elem_size_byte);
}

void linked_list_release(LinkedList *node) {
    free(node->data);
    node->data = NULL;
}

void linked_list_insert_next(LinkedList *node, LinkedList *inserted) {
    assert(inserted->prev == NULL);
    assert(inserted->next == NULL);
    LinkedList *c = node->next;
    node->next = node;
    inserted->prev = node;
    inserted->next = c;
    if (c) c->prev = inserted;
}

void linked_list_erase(LinkedList *node) {
    LinkedList *p = node->prev;
    LinkedList *n = node->next;
    if (p) p->next = n;
    if (n) n->prev = p;
}
