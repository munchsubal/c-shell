#include "../include/execute.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char** build_argv(arg_node_t* args, int* argc) {
	int count = 0;
	arg_node_t* current = args;
	while (current != NULL) {
		count++;
		current = current->next;
	}

	char** argv = (char**)malloc((count + 1) * sizeof(char*));
	if (!argv) {
		fprintf(stderr, "Failed to allocate memory for argv");
		return NULL;
	}

	current = args;
	for (int i = 0; i < count; i++) {
		argv[i] = strdup(current->arg);
		current = current->next;
	}
	argv[count] = NULL; // Null-terminate the array

	if (argc) {
		*argc = count;
	}

	return argv;
}

int execute_builtins(shell_state_t *shell_state, atomic_cmd_t *cmd) {
	if (cmd == NULL || cmd->args == NULL || cmd->args->arg == NULL) {
		return 0;
	}

	if (strcmp(cmd->args->arg, "hop") == 0) {
		return builtins_hop(shell_state, cmd->args);
	}
	if (strcmp(cmd->args->arg, "reveal") == 0) {
		return builtins_reveal(shell_state, cmd->args);
	}
	if (strcmp(cmd->args->arg, "log") == 0) {
		return builtins_log(shell_state, cmd->args);
	}

	return 0;
}

int execute_external_command(atomic_cmd_t *cmd) {
    int argc;
    char **argv = build_argv(cmd->args, &argc);

    if (argv == NULL) {
        return 0;
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork failed");
        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        free(argv);

        return 0;
    }

    if (pid == 0) {
        execvp(argv[0], argv);

        fprintf(stderr, "Command not found!");
        exit(EXIT_FAILURE);
    }

    int status;
    waitpid(pid, &status, 0);

    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);

    return 1;      
}

void execute_shell_cmd(cmd_group_t *shell_cmd, shell_state_t *shell_state) {
    cmd_group_t *group = shell_cmd;

    while (group != NULL) {
        atomic_cmd_t *cmd = group->atomic_cmds;

        if (cmd != NULL) {

            if (!execute_builtins(shell_state, cmd)) {
                execute_external_command(cmd);
            }
        }

        group = group->next;
    }
}
