#ifndef INCLUDE_GUARD_COMMON_H
#define INCLUDE_GUARD_COMMON_H

#include <stddef.h>
#include <stdint.h>

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


#define WRITE_INT_LE(value, dst)\
        do {\
            for (size_t i = 0; i < sizeof(value); i++) {\
                dst[i] = (value >> (i * 8)) & ((1 << 8) - 1);\
            }\
            dst += sizeof(value);\
        } while (0)
#define WRITE_ENUM_LE(value, dst)\
        do {\
            int32_t tmp = value;\
            WRITE_INT_LE(tmp, dst);\
        } while (0)


#define READ_INT_LE(src, dst)\
        do {\
            *dst = 0;\
            for (size_t i = 0; i < sizeof(*dst); i++) {\
                *dst |= src[i] << (i * 8);\
            }\
            src += sizeof(*dst);\
        } while (0)

#define READ_ENUM_LE(src, dst)\
        do {\
            int32_t tmp = 0;\
            READ_INT_LE(src, &tmp);\
            *dst = tmp;\
        } while (0)

#endif