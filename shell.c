/*
The microshell project 'KUSh' - KUCIA's Shell
made by Artur Pozniak aka KUCIA

TODO: anything...
*/


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

#define READ_BUFFER_SIZE 1024

// helper structures and objects
typedef struct{
    const char* WHITE;
    const char* BLACK;
    const char* RED;
    const char* GREEN;
    const char* PURPLE;
    const char* YELLOW;
    const char* CYAN;
} colors_t;

colors_t Colors = {"\x1b[0;37m", "\x1b[0;30m", "\x1b[0;31m", "\x1b[0;32m", "\x1b[0;35m", "\x1b[0;33m", "\x1b[0;36m"};

// helper functions and methods
void cprint(const char* text, const char* color){
    // function used to print text with certain color
    printf("%s%s%s", color, text, Colors.WHITE);
}

void cprintnl(const char* text, const char* color){
    // same as cprint, but prints an end of the line
    cprint(text, color);
    printf("\n");
}

void error_message(const char* text) {
    printf("[");
    cprint("ERROR", Colors.RED);
    printf("]: %s", text);
}

void print_greetings(void){
    printf(" \n");
    cprintnl(R"edge(____    __    ____  _______  __        ______   ______   .___  ___.  _______ )edge", Colors.PURPLE);
    cprintnl(R"edge(\   \  /  \  /   / |   ____||  |      /      | /  __  \  |   \/   | |   ____|)edge", Colors.PURPLE);
    cprintnl(R"edge( \   \/    \/   /  |  |__   |  |     |  ,----'|  |  |  | |  \  /  | |  |__   )edge", Colors.PURPLE);
    cprintnl(R"edge(  \            /   |   __|  |  |     |  |     |  |  |  | |  |\/|  | |   __|  )edge", Colors.PURPLE);
    cprintnl(R"edge(   \    /\    /    |  |____ |  `----.|  `----.|  `--'  | |  |  |  | |  |____ )edge", Colors.PURPLE);
    cprintnl(R"edge(    \__/  \__/     |_______||_______| \______| \______/  |__|  |__| |_______|)edge", Colors.PURPLE);
    cprintnl(R"edge(                                                                             )edge", Colors.PURPLE);
    cprintnl(R"edge(.___________.  ______        __  ___  __    __       _______. __    __    __ )edge", Colors.PURPLE);
    cprintnl(R"edge(|           | /  __  \      |  |/  / |  |  |  |     /       ||  |  |  |  |  |)edge", Colors.PURPLE);
    cprintnl(R"edge(`---|  |----`|  |  |  |     |  '  /  |  |  |  |    |   (----`|  |__|  |  |  |)edge", Colors.PURPLE);
    cprintnl(R"edge(    |  |     |  |  |  |     |    <   |  |  |  |     \   \    |   __   |  |  |)edge", Colors.PURPLE);
    cprintnl(R"edge(    |  |     |  `--'  |     |  .  \  |  `--'  | .----)   |   |  |  |  |  |__|)edge", Colors.PURPLE);
    cprintnl(R"edge(    |__|      \______/      |__|\__\  \______/  |_______/    |__|  |__|  (__))edge", Colors.PURPLE);
    cprintnl(R"edge(                                                                             )edge", Colors.PURPLE);
    cprintnl(R"edge(-----------------------------------------------------------------------------)edge", Colors.PURPLE);
    printf(" \n");
}

int kush_loop() {

    // allocating a buffer to read the user input
    char* read_buff = malloc(READ_BUFFER_SIZE);
    if (read_buff == nullptr) {
        error_message("Unable to allocate memory for read buffer");
        return -1;
    }

    return 0;
}


int main(void){
    setlocale(LC_ALL, "");  // Set locale to the user's default
    print_greetings();

    // entering the loop
    return kush_loop();
}