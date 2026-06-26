#include "../include/execute.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
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

void handle_input_redirection(const char* input_redir) {
    if (input_redir != NULL) {
        int fd = open(input_redir, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "No such file or directory\n");
            return;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    return;
}

void handle_output_redirection(const char* output_redir, int append) {
    if (output_redir != NULL) {
        int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
        int fd = open(output_redir, flags, 0644);
        if (fd < 0) {
            fprintf(stderr, "Failed to open output file\n");
            return;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    return;
}

int isbuiltin(const char* cmd) {
    return (strcmp(cmd, "hop") == 0 || strcmp(cmd, "reveal") == 0 || strcmp(cmd, "log") == 0);
}

void execute_builtins(shell_state_t *shell_state, atomic_cmd_t *cmd) {
	if (cmd == NULL || cmd->args == NULL || cmd->args->arg == NULL) {
		return;
	}

	if (strcmp(cmd->args->arg, "hop") == 0) {
		builtins_hop(shell_state, cmd->args);
        return;
	}
	if (strcmp(cmd->args->arg, "reveal") == 0) {
		builtins_reveal(shell_state, cmd->args);
        return;
	}
	if (strcmp(cmd->args->arg, "log") == 0) {
		builtins_log(shell_state, cmd->args);
        return;
	}

	return;
}

void execute_external_command(atomic_cmd_t *cmd) {
    int argc;
    char **argv = build_argv(cmd->args, &argc);

    if (argv == NULL) {
        return;
    }

    execvp(argv[0], argv);

    fprintf(stderr, "Command not found!\n");
    exit(EXIT_FAILURE);    
}

void execute_single_command(shell_state_t *shell_state, atomic_cmd_t *cmd) {
    if (isbuiltin(cmd->args->arg)) {

        int saved_in = dup(STDIN_FILENO);
        int saved_out = dup(STDOUT_FILENO);

        handle_input_redirection(cmd->input_redir);
        handle_output_redirection(cmd->output_redir, cmd->append_output);   

        execute_builtins(shell_state, cmd);

        fflush(stdout);

        dup2(saved_in, STDIN_FILENO);
        dup2(saved_out, STDOUT_FILENO);

        close(saved_in);
        close(saved_out);

        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {

        handle_input_redirection(cmd->input_redir);
        handle_output_redirection(cmd->output_redir, cmd->append_output);

        execute_external_command(cmd);
    }

    waitpid(pid, NULL, 0);

    return;
}

void execute_atomic(shell_state_t *shell_state, atomic_cmd_t *cmd) {
    if (cmd == NULL) {
        return;
    }

    if (isbuiltin(cmd->args->arg)) {
        execute_builtins(shell_state, cmd);
    } 
    else {
        execute_external_command(cmd);
    }
}

void handle_pipelines(shell_state_t *shell_state, atomic_cmd_t *cmd) {

    const int MAX_PIPELINE_CMDS = 100;

    int prev_read_fd = -1;

    pid_t pids[MAX_PIPELINE_CMDS];
    int cmd_count = 0;

    atomic_cmd_t *curr = cmd;

    while (curr != NULL) {

        int pipefd[2];

        if (curr->next != NULL) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return;
            }
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            return;
        }

        if (pid == 0) {

            // ---------- stdin ----------

            if (prev_read_fd != -1) {
                dup2(prev_read_fd, STDIN_FILENO);
                close(prev_read_fd);
            }
            else {
                handle_input_redirection(curr->input_redir);
            }

            // ---------- stdout ----------

            if (curr->next != NULL) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                close(pipefd[0]);
            }
            else {
                handle_output_redirection(curr->output_redir, curr->append_output);
            }

            execute_atomic(shell_state, curr);

            fflush(stdout);
            exit(EXIT_SUCCESS);
        }

        // Parent

        pids[cmd_count++] = pid;

        if (prev_read_fd != -1) {
            close(prev_read_fd);
        }

        if (curr->next != NULL) {
            close(pipefd[1]);
            prev_read_fd = pipefd[0];
        }
        else {
            prev_read_fd = -1;
        }

        curr = curr->next;
    }

    if (prev_read_fd != -1) {
        close(prev_read_fd);
    }

    for (int i = 0; i < cmd_count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

void execute_shell_cmd(cmd_group_t *shell_cmd, shell_state_t *shell_state) {
    cmd_group_t *group = shell_cmd;

    while (group != NULL) {
        atomic_cmd_t *cmd = group->atomic_cmds;

        if (cmd != NULL && cmd->next == NULL) {
            execute_single_command(shell_state, cmd);
        }
        else {
            handle_pipelines(shell_state, cmd);
        }

        group = group->next;
    }
}
