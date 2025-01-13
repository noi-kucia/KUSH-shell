/*
The microshell project 'KUSh' - KUCIA's Shell
made by Artur Pozniak aka KUCIA

BAJERY:
1) when path length in prompt exceeds maximal value, it's getting truncated;
   Also the home path is replaced by ~ automatically
2) strings in quotes allows to use raw strings (both ' and ")
3) escape character (backslash) support
4) strongly protected from allocation errors
5) error emphasizing
6) built-in history and  navigating in it with arrows
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
#include "print_utils.h"
#include "tokens.h"
#include "executor.h"
#include "shell.h"

#define READ_BUFFER_SIZE 1024

const char* HOME_PATH = "";  // is set in prepare function
size_t history_size = 0;
char *history[HISTORY_MAX_SIZE];

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
    char *path = malloc(path_max_size);
    char *buff = malloc(buff_size);
    if (path==nullptr||buff==nullptr) {
        error_message("Failed to allocate memory for the shell command prompt");
        return;
    }
    for (uint8_t iteration=1;iteration<=11;iteration++){
        char *res = getcwd(buff, buff_size);
        if (res==NULL && errno==ERANGE) {// if the path is longer than max size
            buff_size *= 2;
            char *buff_rlctd = realloc(buff, buff_size);
            if (buff_rlctd==nullptr) {
                error_message("Failed to allocate memory for the buffer");
                free(buff);
                break;
            }
            buff = buff_rlctd;
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
    /* Reads the user input character by character updating the buffer in live mode.
     * Returns a number of characters in the buffer when \n character is entered.
     * If a special key is pressed (like arrows), it won't be added to the buffer
     * but the corresponding callback function will be invoked.
     */
    char key;
    size_t charc = 0;
    while (true) {
        disable_icanon();
        read(STDIN_FILENO, &key, 1);  // reading the character
        disable_icanon();
        if (key==LINE_END) {
            key = '\0';
        }

        // Processing special keys.
        // These symbols won't be displayed and added to the buffer
        bool is_special = true;
        switch (key) {
            case DEL:
                if (charc <= 0) break;  // to prevent erasing of a non-user printed text
                printf("\b \b");
                charc--;
            break;
            case ESC:
                disable_icanon();
                read(STDIN_FILENO, &key, 1);
                if (key!=LSQUARE) {
                    error_message("Unknown escape sequence on input (no square brace was found after \033)");
                    exit(127);
                }
                read(STDIN_FILENO, &key, 1);
                disable_icanon();
                switch (key) {
                    case ARROW_UP:
                        // TODO: history

                    break;
                    case ARROW_DOWN:
                        // TODO: history
                    break;
                    default:
                        // error_message("Unknown escape sequence on input (after \\033[)");
                        break;
                }
                break;
            case TAB:
                // TODO: autocomletion
            break;
            default:
                is_special = false;
            break;
        }
        if (is_special) continue;

        // appending it to the buffer
        if (charc >= *buffer_size) {
            perror("Buffer size is to small, unable to read a character to it");
            exit(72);
        }
        buff[charc++] = (char)key;
        printf("%c", key); // displaying it
        if (key=='\0') {
            // appending command to the history
            if (history_size >= HISTORY_MAX_SIZE) {
                free(history[0]);
                for (int i=1;i<HISTORY_MAX_SIZE;i++) history[i-1] = history[i];
                history_size--;
            }
            char *command = malloc(charc+1);
            strncpy(command, buff, charc);
            command[charc] = '\0';
            history[history_size++] = command;

            // printing nl and returning
            printf("\n");
            return charc;
        }
    }
}

int kush_loop() {
    // allocating a buffer to read the user input
    size_t buffer_size = READ_BUFFER_SIZE;
    size_t chars_read = 0;
    char *read_buff = malloc(buffer_size);
    if (read_buff == nullptr) {
        error_message("Unable to allocate memory for read buffer");
        return -1;
    }

    while (true) {
        prompt(); // printing the prompt
        chars_read = read_user_command(read_buff, &buffer_size); // reading command
        struct token ** tokens = get_tokens_safe(read_buff); // tokenizing command
        if (tokens){
            execute_sequence(tokens); // executing command
            free_sequence(tokens);  // freeing memory
        }
    }

    return 0;
}

void prepare() {
    HOME_PATH = getenv("HOME");
    history_size = 0;
}

int main(void){
    setlocale(LC_ALL, "");  // Set locale to the user's default
    prepare();
    print_greetings();

    // entering the loop
    return kush_loop();
}