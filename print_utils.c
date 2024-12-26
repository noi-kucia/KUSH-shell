//
// Created by kucia on 12/26/24.
//

#include "print_utils.h"

colors_t Colors = {"\x1b[0;37m",
    "\x1b[0;30m", "\x1b[0;31m", "\x1b[0;32m", "\x1b[0;35m", "\x1b[0;33m",
    "\x1b[0;36m", "\x1b[34m", "\x1b[0;37m"};

void cprint(const char* text, const char* color){
    // function used to print text with certain color
    printf("%s%s%s", color, text, Colors.CLEAR);
}

void cprintnl(const char* text, const char* color){
    // same as cprint, but prints an end of the line
    cprint(text, color);
    printf("\n");
}