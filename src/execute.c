#include "../include/execute.h"

#include <string.h>

int execute_builtins(shell_state_t *shell_state, atomic_cmd_t *cmd) {
	if (cmd == NULL || cmd->args == NULL || cmd->args->arg == NULL) {
		return 0;
	}

	if (strcmp(cmd->args->arg, "hop") == 0) {
		return builtins_hop(shell_state, cmd->args);
	}

	return 0;
}

void execute_shell_cmd(cmd_group_t *shell_cmd, shell_state_t *shell_state) {
    // For now, just execute builtins for the first atomic command of the first command group
    if (shell_cmd != NULL && shell_cmd->atomic_cmds != NULL) {
        if (execute_builtins(shell_state, shell_cmd->atomic_cmds)) {
            return;
        }
    }
}
