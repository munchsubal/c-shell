#ifndef BUILTINS_H
#define BUILTINS_H

#include "command.h"
#include <stdbool.h>
#define HISTORY_LIMIT 15

typedef struct {
    char* prev_dir;
    bool prev_dir_valid;
} shell_state_t;

int builtins_hop(shell_state_t* shell_state, const arg_node_t* args);
int builtins_reveal(shell_state_t* shell_state, const arg_node_t* args);
bool contains_log_command(cmd_group_t* shell_cmd);
int add_command_to_history(const char* command);
int builtins_log(shell_state_t* shell_state, const arg_node_t* args);

#endif // BUILTINS_H