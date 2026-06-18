#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "command.h"
typedef struct parser_state_t {
    token_t* current_token;
} parser_state_t;

atomic_cmd_t* parse_atomic_cmd(parser_state_t* state);
cmd_group_t* parse_cmd_group(parser_state_t* state);
cmd_group_t* parse_shell_cmd(token_t* token_stream);
void debug_print_shell_cmd(cmd_group_t* groups);

#endif // PARSER_H