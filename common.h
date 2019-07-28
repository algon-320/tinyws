#ifndef INCLUDE_GUARD_COMMON_H
#define INCLUDE_GUARD_COMMON_H

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

#endif