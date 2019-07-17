#ifndef INCLUDE_GUARD_WINDOW_H
#define INCLUDE_GUARD_WINDOW_H

#include "basic_structures.h"

struct Window {
    int id;
    struct Window **children;
    Size size;
};

#endif