#ifndef INCLUDE_GUARD_COMMON_H
#define INCLUDE_GUARD_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#define SDL_CALL_NONNEG(func_name, ...)\
        do {\
            if (func_name(__VA_ARGS__) < 0) {\
                fprintf(stderr, __FILE__ ": " #func_name " Error: %s\n", SDL_GetError());\
                return -1;\
            }\
        } while (0)

#define OFFSET_OF(type, member)\
        ((size_t)&((type *)0)->member)
#define CONTAINNER_OF(ptr, containner, member)\
        ((containner *)((char *)ptr - OFFSET_OF(containner, member)))


// value: any_int_type, dst: uint8_t**
#define WRITE_INT_LE(value, dst)\
        do {\
            for (size_t i = 0; i < sizeof(value); i++) {\
                (*(dst))[i] = ((value) >> (i * 8)) & ((1 << 8) - 1);\
            }\
            (*(dst)) += sizeof(value);\
        } while (0)
// value: enum_value, dst: uint8_t**
#define WRITE_ENUM_LE(value, dst)\
        do {\
            int32_t tmp = value;\
            WRITE_INT_LE(tmp, dst);\
        } while (0)

// src: uint8_t**, dst: any_int_type*
#define READ_INT_LE(src, dst)\
        do {\
            *(dst) = 0;\
            for (size_t i = 0; i < sizeof(*(dst)); i++) {\
                *(dst) |= (*(src))[i] << (i * 8);\
            }\
            *(src) += sizeof(*(dst));\
        } while (0)

// src: uint8_t**, dst: enum_value*
#define READ_ENUM_LE(src, dst)\
        do {\
            int32_t tmp = 0;\
            READ_INT_LE(src, &tmp);\
            *(dst) = tmp;\
        } while (0)

// mutex: pthread_mutex_t*
#define lock_mutex(mutex) do {\
    int r = pthread_mutex_lock(mutex);\
    if (r != 0) {\
        fprintf(stderr, "can not lock\n");\
        assert(false);\
    }\
} while (0)
// mutex: pthread_mutex_t*
#define unlock_mutex(mutex) do {\
    int r = pthread_mutex_unlock(mutex);\
    if (r != 0) {\
        fprintf(stderr, "can not lock\n");\
        assert(false);\
    }\
} while (0)

#define PANIC(...) do { fprintf(stderr, "panic: " __VA_ARGS__); exit(1); } while (0)

#ifdef NDEBUG
    #define debugprint(...)
#else
    #define debugprint(...) do { fprintf(stderr, "\x1b[33;1m"); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\x1b[0m"); } while (0)
#endif

typedef int32_t client_id_t;
extern const client_id_t CLIENT_ID_INVALID;

typedef int32_t window_id_t;
extern const window_id_t WINDOW_ID_INVALID;

#endif