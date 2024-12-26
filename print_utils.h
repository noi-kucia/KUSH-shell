//
// Created by kucia on 12/26/24.
//

#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

#include <stdio.h>

typedef struct{
    const char* WHITE;
    const char* BLACK;
    const char* RED;
    const char* GREEN;
    const char* PURPLE;
    const char* YELLOW;
    const char* CYAN;
    const char* BLUE;
    const char* CLEAR;
} colors_t;

extern colors_t Colors;

void cprintnl(const char* text, const char* color);
void cprint(const char* text, const char* color);


#endif //PRINT_UTILS_H
