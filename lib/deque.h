#ifndef INCLUDE_GUARD_DEQUE_H
#define INCLUDE_GUARD_DEQUE_H

#include <stddef.h>

typedef struct Deque_ {
    void *data;
    size_t elem_size_byte;
    size_t capacity;
    size_t front_idx;
    size_t end_idx;
} Deque;

Deque deque_new(size_t num, size_t elem_size_byte);
Deque deque_new_filled_with(size_t num, size_t elem_size_byte, void *elem);
Deque deque_new_with_capacity(size_t num, size_t elem_size_byte, size_t capacity);

void deque_free(Deque *deq);

size_t deque_size(Deque *deq);
void *deque_at(Deque *deq, size_t idx);
void *deque_front(Deque *deq);
void *deque_back(Deque *deq);
void deque_push_back(Deque *deq, void *elem);
void deque_pop_back(Deque *deq);
void deque_push_front(Deque *deq, void *elem);
void deque_pop_front(Deque *deq);

#endif