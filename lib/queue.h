#ifndef INCLUDE_GUARD_QUEUE_H
#define INCLUDE_GUARD_QUEUE_H

#include "deque.h"

#define QUEUE_DEFAULT_CAPACITY (16)

typedef Deque Queue;

#define queue_new(elem_size_byte) deque_new_with_capacity(0, elem_size_byte, QUEUE_DEFAULT_CAPACITY)
#define queue_free deque_free
#define queue_pop deque_pop_front
#define queue_push deque_push_back
#define queue_front deque_front
#define queue_size deque_size
#define queue_empty(q) (queue_size(q) == 0)

#endif