//
// Created by kucia on 1/7/25.
//

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

#define VERSION "0.6.0"
#define LINE_END '\n'

#define ESC 27
#define DEL 127
#define LSQUARE '['
#define ARROW_LEFT 'D'
#define ARROW_UP 'A'
#define ARROW_RIGHT 'C'
#define ARROW_DOWN 'B'
#define TAB '\t'

#define HISTORY_MAX_SIZE 50
extern size_t history_size;
extern bool in_history_mode;
extern char *history[HISTORY_MAX_SIZE];

extern bool in_autocomplete_mode;
extern uint16_t ac_ind;
extern uint16_t previously_printed_chars;
extern size_t ac_entries;
extern char **ac_names;
extern char *ac_prefix;


#endif //SHELL_H
