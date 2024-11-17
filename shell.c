/*
The microshell project
made by Artur Pozniak

TODO: anything...
*/


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

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

void print_greetings(void){
    printf(" \n");
    cprintnl(R"edge( ______________________________________________________   )edge", Colors.PURPLE);
    cprintnl(R"edge(/_____/_____/_____/_____/_____/_____/_____/_____/_____/   )edge", Colors.PURPLE);
    cprintnl(R"edge( _      _____  / /________  ____ ___  ___     / /_____  _ )edge", Colors.PURPLE);
    cprintnl(R"edge(| | /| / / _ \/ / ___/ __ \/ __ `__ \/ _ \   / __/ __ \(_))edge", Colors.PURPLE);
    cprintnl(R"edge(| |/ |/ /  __/ / /__/ /_/ / / / / / /  __/  / /_/ /_/ /   )edge", Colors.PURPLE);
    cprintnl(R"edge(|__/|__/\___/_/\___/\____/_/ /_/ /_/\___/   \__/\____(_)  )edge", Colors.PURPLE);
    cprintnl(R"edge(                   / //_/ ___// / / /                     )edge", Colors.PURPLE);
    cprintnl(R"edge(                  / ,<  \__ \/ /_/ /                      )edge", Colors.PURPLE);
    cprintnl(R"edge(                 / /| |___/ / __  /                       )edge", Colors.PURPLE);
    cprintnl(R"edge(________________/_/_|_/____/_/_/_/_______________         )edge", Colors.PURPLE);
    cprintnl(R"edge(/_____/_____/_____/_____/_____/_____/_____/_____/         )edge", Colors.PURPLE);
}

int main(void){
    printf("\n");
    print_greetings();
    return 0;
}