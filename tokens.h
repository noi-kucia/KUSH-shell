//
// Created by kucia on 12/1/24.
//

#ifndef TOKENS_H
#define TOKENS_H

#include <stdint.h>

// defining types
typedef char tchar_t;  // type of token character
enum token_types{
    token_error,
    token_unknown,
    token_unfinished,
    token_empty,
    token_commandterm,
    token_semicolon,
    token_pipe,
    token_inredir,
    token_inredirap,
    token_outredir,
    token_outredirap,
    token_end
};


struct token{
    enum token_types type;
    const tchar_t *src;
    uint16_t length;
};

struct token next_token_safe(const tchar_t *);
struct token **get_tokens_safe(const tchar_t *);
struct token **get_pipe_segment(struct token **prev_segment);



#endif //TOKENS_H
