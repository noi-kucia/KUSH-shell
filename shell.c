/*
The microshell project 'KUSh' - KUCIA's Shell
made by Artur Pozniak aka KUCIA

BAJERY:
1) when path length in prompt exceeds maximal value, it's getting truncated;
   Also the home path is replaced by ~ automatically
2) strings in quotes allows to use raw strings (both ' and ")
3) escape character (backslash) support
*/

// standard libs
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>
#include <stdbool.h>
#include <errno.h>

// local includes
#include "tokens.h"
#include "executor.h"

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
    const char* BLUE;
    const char* CLEAR;
} colors_t;

colors_t Colors = {"\x1b[0;37m",
    "\x1b[0;30m", "\x1b[0;31m", "\x1b[0;32m", "\x1b[0;35m", "\x1b[0;33m",
    "\x1b[0;36m", "\x1b[34m", "\x1b[0;37m"};
const char* HOME_PATH = "";  // is set in prepare function; does not contain last slash

// helper functions and methods
void cprint(const char* text, const char* color){
    // function used to print text with certain color
    printf("%s%s%s", color, text, Colors.CLEAR);
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
    cprintnl(R"edge(                                                                             )edge", Colors.PURPLE);
    cprintnl(R"edge(       >/< type help to get information about functionality >/<              )edge", Colors.GREEN);
    cprint(R"edge(                  >/< made by )edge", Colors.GREEN);
    cprint("Artur Pozniak", Colors.YELLOW);
    cprintnl(" >/<                                                            ", Colors.GREEN);
    printf(" \n");
}

void prompt() {
    /*     prints a prompt */

    // getting user and host names
    char* username = getlogin();
    char* hostname = malloc(64);
    gethostname(hostname, 64);

    // getting a path into the buffer
    uint16_t path_max_size = 64;
    uint_fast16_t buff_size = path_max_size;
    char* path = malloc(path_max_size);
    char* buff = malloc(buff_size);
    if (path==nullptr||buff==nullptr) {
        error_message("Failed to allocate memory for the shell command prompt");
        return;
    }
    for (uint8_t iteration=1;iteration<=11;iteration++){
        char* res = getcwd(buff, buff_size);
        if (res==NULL && errno==ERANGE) {// if the path is longer than max size
            buff_size *= 2;
            buff = realloc(buff, buff_size);
            if (buff==nullptr) {
                error_message("Failed to allocate memory for the buffer");
                break;
            }
        }
        else {
            break;
        };
    }

    // replacing a home path by ~
    if (strncmp(HOME_PATH, buff, strlen(HOME_PATH))==0){
        buff[0] = '~';
        strlcpy(buff+1, buff+strlen(HOME_PATH), buff_size-1);
    }

    // truncating path if needed or just copying it
    if (strlen(buff)>path_max_size) {
        strncpy(path, "...", path_max_size);
        const char *start_index = strrchr(buff, '/');
        strncpy(path+3, start_index, path_max_size-4);
    }
    else strncpy(path, buff, path_max_size);





    // printing a result
    printf("%s@%s-> %s%s%s$", username, hostname, Colors.BLUE, path, Colors.CLEAR);

    free(buff);
    free(path);
}

size_t  read_user_command(char *buff, size_t *buffer_size) {
    /* reads the user input */
    return getline(&buff, buffer_size, stdin);;
}

int kush_loop() {
    // allocating a buffer to read the user input
    size_t buffer_size = READ_BUFFER_SIZE;
    size_t input_len = 0;
    char* read_buff = malloc(buffer_size);
    if (read_buff == nullptr) {
        error_message("Unable to allocate memory for read buffer");
        return -1;
    }

    while (true) {
        prompt(); // printing the prompt
        input_len = read_user_command(read_buff, &buffer_size); // reading command
        read_buff[input_len-1] = '\0';  // removing the last nl character
        struct token ** tokens = get_tokens(read_buff); // tokenizing command
        execute_sequence(tokens); // executing command
        free_sequence(tokens);  // freeing memory
    }

    return 0;
}

void prepare() {
    HOME_PATH = getenv("HOME");
}

int main(void){
    setlocale(LC_ALL, "");  // Set locale to the user's default
    prepare();
    print_greetings();

    // entering the loop
    return kush_loop();
}