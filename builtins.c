#include "builtins.h"
#include "command.h"
#include "shell.h"
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

extern char HOME_DIR[PATH_MAX];

int change_directory(shell_state_t* shell_state, const char* path) {
    char current_dir[PATH_MAX];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        fprintf(stderr, "Error getting current directory\n");
        return 0;
    }

    if (chdir(path) != 0) {
        fprintf(stderr, "No such directory!\n");
        return 0;
    }

    // Update previous directory
    free(shell_state->prev_dir);
    shell_state->prev_dir = strdup(current_dir);
    shell_state->prev_dir_valid = true;

    return 1;
}

int builtins_hop(shell_state_t* shell_state, const arg_node_t* args) {
    args = args->next; // Skip the command name

    if (args == NULL) {
        return change_directory(shell_state, HOME_DIR);
    }

    while (args != NULL) {
        if (args->arg[0] == '~') {
            char path[PATH_MAX]; 
            snprintf(path, sizeof(path), "%s%s", HOME_DIR, args->arg + 1); // Handle paths starting with ~
            if (!change_directory(shell_state, path)) {
                return 0;
            }
        } 
        else if (strcmp(args->arg, ".") == 0) { 
            args = args->next; // Skip the current directory argument
            continue; // Do nothing
        } 
        else if (strcmp(args->arg, "..") == 0) {
            if (!change_directory(shell_state, "..")) {
                return 0;
            }
        } 
        else if (strcmp(args->arg, "-") == 0) {
            if (!shell_state->prev_dir_valid) {
                args = args->next; // Skip the argument
                continue; //Do nothihng
            }
            if (!change_directory(shell_state, shell_state->prev_dir)) {
                return 0;
            }
        }
        else {
            if (!change_directory(shell_state, args->arg)) { // Absolute or Relative path
                return 0;
            }
        }
        args = args->next;
    }

    return 1;
}


