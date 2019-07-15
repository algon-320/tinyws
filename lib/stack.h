#ifndef INCLUDE_GUARD_STACK_H
#define INCLUDE_GUARD_STACK_H

#include "deque.h"

#define STACK_DEFAULT_CAPACITY (16)

typedef Deque Stack;

#define stack_new(elem_size_byte) deque_new_with_capacity(0, elem_size_byte, STACK_DEFAULT_CAPACITY)
#define stack_free deque_free
#define stack_pop deque_pop_back
#define stack_push deque_push_back
#define stack_top deque_back
#define stack_size deque_size
#define stack_empty(q) (stack_size(q) == 0)

#endif