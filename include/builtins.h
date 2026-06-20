#ifndef BUILTINS_H
#define BUILTINS_H

#include "command.h"
#include <stdbool.h>

typedef struct {
    char* prev_dir;
    bool prev_dir_valid;
} shell_state_t;

int builtins_hop(shell_state_t* shell_state, const arg_node_t* args);
int builtins_reveal(shell_state_t* shell_state, const arg_node_t* args);

#endif // BUILTINS_H