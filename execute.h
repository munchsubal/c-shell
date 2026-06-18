#ifndef EXECUTE_H
#define EXECUTE_H

#include "command.h"
#include "builtins.h"

int execute_builtins(shell_state_t *shell_state, atomic_cmd_t *cmd);
void execute_shell_cmd(cmd_group_t *shell_cmd, shell_state_t *shell_state);

#endif // EXECUTE_H
