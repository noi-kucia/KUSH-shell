//
// Created by kucia on 12/26/24.
//

#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

#include <stdio.h>
#include <string.h>
#include "shell.h"
#include <termios.h>

typedef struct{
    const char* WHITE;
    const char* BLACK;
    const char* RED;
    const char* GREEN;
    const char* PURPLE;
    const char* YELLOW;
    const char* CYAN;
    const char* BLUE;
    const char* ORANGE;
    const char* CLEAR;
} colors_t;

extern colors_t Colors;

void cprintnl(const char *text, const char *color);
void cprint(const char *text, const char *color);
void error_message(const char *mesg);
void error_emph_message(const char *mesg, __uint16_t start, __uint16_t end);
void error_emph_prefix(const char *prefix, const char *mesg, __uint16_t start, __uint16_t end);
void disable_icanon();
void enable_icanon();
void erase_terminal(size_t n);


#endif //PRINT_UTILS_H
