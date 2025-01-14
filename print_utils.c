//
// Created by kucia on 12/26/24.
//

#include "print_utils.h"

colors_t Colors = {"\x1b[0;37m",
    "\x1b[0;30m", "\x1b[0;31m", "\x1b[0;32m", "\x1b[0;35m", "\x1b[1;33m",
    "\x1b[0;36m", "\x1b[34m","\x1b[0;33m", "\x1b[0;37m"};

void cprint(const char* text, const char* color){
    // function used to print text with certain color
    printf("%s%s%s", color, text, Colors.CLEAR);
}

void cprintnl(const char* text, const char* color){
    // same as cprint, but prints an end of the line
    cprint(text, color);
    printf("\n");
}

void error_message(const char *mesg) {
    /* just prints an error like
     * [ERROR]: some error in red color
     */
    printf("[");
    cprint("ERROR", Colors.RED);
    printf("]: %s\n", mesg);
}

void error_emph_message(const char *mesg, __uint16_t start, __uint16_t end) {
    /* similar to error_message, but will also emphasize characters from start to end like
     * [ERROR]: some error in red
     * ~~~~~~~~~~~~~~~~~~~~^^^^~~
     */

    __uint8_t preflen = 9;
    error_message(mesg);
    for (int i=0;i<preflen+strlen(mesg);i++) {
        printf((i-preflen>=start && i-preflen<end)? "\x1b[0;31m^\x1b[0;37m":"~");
    }
    printf("\n");
}

void error_emph_prefix(const char *prefix, const char *mesg, __uint16_t start, __uint16_t end) {
    /* Works like an error_emph_message, but the message is split into prefix and an actual message.
     * Start and end indexes will be calculated relative to message length regardless of prefix length.
     */
    unsigned int preflen = strlen(prefix);
    char text[preflen+strlen(mesg)+1];
    sprintf(text, "%s%s\0", prefix, mesg);
    error_emph_message(text, preflen+start, preflen+end);
}

struct termios old = {0};

void disable_icanon() {
    fflush(stdout);
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
}

void enable_icanon() {
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
}

void erase_terminal(size_t n) {
    /* erases n characters from terminal */
    for (int i=0;i<n;i++) printf("\b \b");
}