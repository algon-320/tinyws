#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define ASSERT_EQ_INT(a, b) do {\
    if ((a) != (b)) {\
        fprintf(stderr,\
            "\tAssertion `%s` == `%s` failed (%s:%d)\n\t\ta: %lld\n\t\tb: %lld\n",\
            #a, #b, __FILE__, __LINE__, (long long)a, (long long)b); all_passed = 0; test_case_passed = 0;\
    }\
} while (0)

#define ASSERT_EQ_PTR(a, b) do {\
    if ((a) != (b)) {\
        fprintf(stderr,\
            "\tAssertion `%s` == `%s` failed (%s:%d)\n\t\ta: %p\n\t\tb: %p\n",\
            #a, #b, __FILE__, __LINE__, (void *)a, (void *)b); all_passed = 0; test_case_passed = 0;\
    }\
} while (0)


int all_passed;

#include "../deque.h"
void deque_print(Deque *deq) {
    printf("[\n");
    for (int i = 0; i < deque_size(deq); ++i) {
        printf("\ti: %d, deque_at(deq, i) = %d\n", i, *(int *)deque_at(deq, i));
    }
    printf("]\n");
}
void test_deque() {
    int test_case_passed = 1;
    printf("test_deque\n");
    {
        Deque deq = deque_new(10, sizeof(int));
            ASSERT_EQ_INT(deque_size(&deq), 10);
    }
    
    {
        Deque deq = deque_new_with_capacity(0, sizeof(int), 100);
            ASSERT_EQ_INT(deque_size(&deq), 0);
            ASSERT_EQ_INT(deq.capacity, 100);
    }

    {
        int fill = 7;
        Deque deq = deque_new_filled_with(10, sizeof(int), &fill);
            ASSERT_EQ_INT(deque_size(&deq), 10);
            for (int i = 0; i < 10; i++) {
                ASSERT_EQ_INT(*(int *)deque_at(&deq, i), 7);
            }
    }

    {
        Deque deq = deque_new_with_capacity(0, sizeof(int), 100);
        int data = 123;
        deque_push_back(&deq, &data);
            ASSERT_EQ_INT(deque_size(&deq), 1);
            ASSERT_EQ_INT(*(int *)deque_at(&deq, 0), data);
        deque_pop_back(&deq);
            ASSERT_EQ_INT(deque_size(&deq), 0);
    }

    {
        Deque deq = deque_new_with_capacity(0, sizeof(int), 4);
        
        int data = 1;
        deque_push_back(&deq, &data);  // 1
            ASSERT_EQ_INT(deque_size(&deq), 1);
            ASSERT_EQ_INT(deq.front_idx, 0);
            ASSERT_EQ_INT(deq.end_idx, 1);
            ASSERT_EQ_INT(*(int *)deque_front(&deq), 1);
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 1);
        
        data += 1;
        deque_push_back(&deq, &data);  // 2
            ASSERT_EQ_INT(deque_size(&deq), 2);
            ASSERT_EQ_INT(deq.front_idx, 0);
            ASSERT_EQ_INT(deq.end_idx, 2);
            ASSERT_EQ_INT(*(int *)deque_front(&deq), 1);
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 2);

        // deque_print(&deq);

            ASSERT_EQ_INT(*(int *)deque_back(&deq), 2);
        deque_pop_back(&deq);
            ASSERT_EQ_INT(deque_size(&deq), 1);
            ASSERT_EQ_INT(deq.front_idx, 0);
            ASSERT_EQ_INT(deq.end_idx, 1);

        data += 1;
        deque_push_back(&deq, &data);  // 3
            ASSERT_EQ_INT(deque_size(&deq), 2);

            ASSERT_EQ_INT(*(int *)deque_front(&deq), 1);
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 3);
        deque_pop_front(&deq);

        // deque_print(&deq);
            ASSERT_EQ_INT(deque_size(&deq), 1);
            ASSERT_EQ_INT(*(int *)deque_front(&deq), 3);
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 3);

        data += 1;
        deque_push_back(&deq, &data);  // 4
            ASSERT_EQ_INT(deque_size(&deq), 2);

        // deque_print(&deq);

        data += 1;
        deque_push_back(&deq, &data);  // 5
            ASSERT_EQ_INT(deque_size(&deq), 3);

        // deque_print(&deq);

            ASSERT_EQ_INT(*(int *)deque_front(&deq), 3);
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 5);
        
        deque_pop_front(&deq);
            ASSERT_EQ_INT(deque_size(&deq), 2);

        data += 1;
        deque_push_back(&deq, &data);  // 6
            ASSERT_EQ_INT(deque_size(&deq), 3);

            ASSERT_EQ_INT(*(int *)deque_front(&deq), 4);
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 6);
        deque_pop_back(&deq);
            ASSERT_EQ_INT(deque_size(&deq), 2);
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 5);
    }

    {
        Deque deq = deque_new_with_capacity(0, sizeof(int), 3);
        int data = 1;
        deque_push_back(&deq, &data);  // 1
        data += 1;
        deque_push_back(&deq, &data);  // 2
        data += 1;
        deque_push_back(&deq, &data);  // 3
        data += 1;
        deque_push_back(&deq, &data);  // 4
        data += 1;
        deque_push_back(&deq, &data);  // 5
        data += 1;
        deque_push_back(&deq, &data);  // 6
        data += 1;
        deque_push_back(&deq, &data);  // 7
        data += 1;
        deque_push_back(&deq, &data);  // 8

            ASSERT_EQ_INT(deque_size(&deq), 8);
            ASSERT_EQ_INT(*(int *)deque_front(&deq), 1);
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 8);
    }

    {
        Deque deq = deque_new_with_capacity(0, sizeof(int), 3);
        int data = 1;
        deque_push_back(&deq, &data);  // 1
        data += 1;
        deque_push_back(&deq, &data);  // 2

        deque_pop_front(&deq);

        data += 1;
        deque_push_back(&deq, &data);  // 3

        deque_pop_front(&deq);
            ASSERT_EQ_INT(deque_size(&deq), 1);
            ASSERT_EQ_INT(deq.capacity, 3);
        
        data += 1;
        deque_push_back(&deq, &data);  // 4
            ASSERT_EQ_INT(deque_size(&deq), 2);
            ASSERT_EQ_INT(deque_at(&deq, 1), (deq.data));
            ASSERT_EQ_INT(*(int *)deque_back(&deq), 4);
            ASSERT_EQ_INT(deq.front_idx, 2);
            ASSERT_EQ_INT(deq.end_idx, 1);

        // deque_print(&deq);

        data += 1;
        deque_push_back(&deq, &data);  // 5

        // deque_print(&deq);
    }

    if (test_case_passed) {
        printf("\tok\n");
    } else {
        printf("\tfailed\n");
    }
}


#include "../queue.h"
void test_queue() {
    int test_case_passed = 1;
    printf("test_queue\n");
    {
        Queue q = queue_new(sizeof(int));

        int data = 1;
        queue_push(&q, &data);  // 1

        data += 1;
        queue_push(&q, &data);  // 2

        data += 1;
        queue_push(&q, &data);  // 3

            ASSERT_EQ_INT(queue_empty(&q), 0);
            ASSERT_EQ_INT(queue_size(&q), 3);
            ASSERT_EQ_INT(*(int *)queue_front(&q), 1);

        queue_pop(&q);
            ASSERT_EQ_INT(queue_size(&q), 2);
            ASSERT_EQ_INT(*(int *)queue_front(&q), 2);

        queue_pop(&q);
            ASSERT_EQ_INT(queue_size(&q), 1);
            ASSERT_EQ_INT(*(int *)queue_front(&q), 3);

        queue_pop(&q);
            ASSERT_EQ_INT(queue_empty(&q), 1);
    }
    
    if (test_case_passed) {
        printf("\tok\n");
    } else {
        printf("\tfailed\n");
    }
}

#include "../stack.h"
void test_stack() {
    int test_case_passed = 1;
    printf("test_stack\n");
    {
        Stack st = stack_new(sizeof(int));
        int data = 1;
        stack_push(&st, &data);
            ASSERT_EQ_INT(stack_size(&st), 1);
            ASSERT_EQ_INT(*(int *)stack_top(&st), 1);

        data += 1;
        stack_push(&st, &data);
            ASSERT_EQ_INT(stack_size(&st), 2);
            ASSERT_EQ_INT(*(int *)stack_top(&st), 2);

        data += 1;
        stack_push(&st, &data);
            ASSERT_EQ_INT(stack_size(&st), 3);
            ASSERT_EQ_INT(*(int *)stack_top(&st), 3);

        stack_pop(&st);
            ASSERT_EQ_INT(stack_size(&st), 2);
            ASSERT_EQ_INT(*(int *)stack_top(&st), 2);
    }

    if (test_case_passed) {
        printf("\tok\n");
    } else {
        printf("\tfailed\n");
    }
}

// void test_() {
//     int test_case_passed = 1;
//     printf("test_\n");
//     {
//         // add test code here
//     }
//     if (test_case_passed) {
//         printf("\tok\n");
//     } else {
//         printf("\tfailed\n");
//     }
// }

int main() {
    all_passed = 1;

    test_deque();
    test_queue();
    test_stack();

    if (all_passed) {
        printf("all passed!\n");
    }
    return 0;
}