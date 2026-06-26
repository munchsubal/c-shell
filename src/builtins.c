#include "../include/builtins.h"
#include "../include/command.h"
#include "../include/shell.h"
#include "../include/execute.h"
#include "../include/token.h"
#include "../include/parser.h"
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>

extern char HOME_DIR[PATH_MAX];
extern char HISTORY_FILE_PATH[PATH_MAX];

int cmp(const void *a, const void *b) {
    const char *s1 = *(const char **)a;
    const char *s2 = *(const char **)b;

    return strcmp(s1, s2);
}

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

bool contains_log_command(cmd_group_t* shell_cmd) {
    cmd_group_t* current_group = shell_cmd;
    while (current_group != NULL) {
        atomic_cmd_t* current_cmd = current_group->atomic_cmds;
        while (current_cmd != NULL) {
            if (current_cmd->args != NULL && current_cmd->args->arg != NULL) {
                if (strcmp(current_cmd->args->arg, "log") == 0) {
                    return true;
                }
            }
            current_cmd = current_cmd->next;
        }
        current_group = current_group->next;
    }
    return false;
}

int add_command_to_history(const char* command) {
    FILE* history_file = fopen(HISTORY_FILE_PATH, "r");

    char* history[HISTORY_LIMIT];
    int count = 0;
    char line[4096];

    if (history_file != NULL) {
        while (fgets(line, sizeof(line), history_file) &&
               count < HISTORY_LIMIT) {
            line[strcspn(line, "\r\n")] = '\0';
            history[count++] = strdup(line);
        }
        fclose(history_file);
    }

    if (count > 0 && strcmp(history[count - 1], command) == 0) {
        for (int i = 0; i < count; i++) {
            free(history[i]);
        }
        return 1;
    }

    if (count == HISTORY_LIMIT) {
        free(history[0]);

        for (int i = 1; i < HISTORY_LIMIT; i++) {
            history[i - 1] = history[i];
        }

        history[HISTORY_LIMIT - 1] = strdup(command);
    }
    else {
        history[count++] = strdup(command);
    }

    history_file = fopen(HISTORY_FILE_PATH, "w");
    if (history_file == NULL) {
        perror("Failed to open history file");

        for (int i = 0; i < count; i++) {
            free(history[i]);
        }

        return 0;
    }

    for (int i = 0; i < count; i++) {
        fprintf(history_file, "%s\n", history[i]);
        free(history[i]);
    }

    fclose(history_file);
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

int builtins_reveal(shell_state_t* shell_state, const arg_node_t* args) { //equivalent to ls
    args = args->next; // Skip the command name

    bool show_hidden = false;
    bool long_format = false;
    bool path_provided = false;
    char* target_path = NULL; // Default to current directory

    while (args != NULL) {
        if (path_provided) {
            fprintf(stderr, "reveal: Invalid Syntax!\n");
            free(target_path);
            return 0;
        }

        if (args->arg[0] == '-' && strlen(args->arg) > 1) {
            for (size_t i = 1; i < strlen(args->arg); ++i) {
                if (args->arg[i] == 'a') {
                    show_hidden = true;
                } 
                else if (args->arg[i] == 'l') {
                    long_format = true;
                } 
                else {
                    fprintf(stderr, "reveal: Invalid syntax!\n");
                    return 0;
                }
            }
        } 
        else {
            free(target_path);
            target_path = strdup(args->arg); // Set the target path to the provided argument
            path_provided = true;
        }
        args = args->next;
    }

    if (target_path == NULL) {
        target_path = strdup("."); // Default to current directory if no path provided
    }
    else if (target_path[0] == '~') {
        char path[PATH_MAX]; 
        snprintf(path, sizeof(path), "%s%s", HOME_DIR, target_path + 1); // Handle paths starting with ~
        free(target_path);
        target_path = strdup(path);
    }
    else if (strcmp(target_path, "-") == 0) {
        if (!shell_state->prev_dir_valid) {
            fprintf(stderr, "No such directory!\n");
            free(target_path);
            return 0;
        }
        free(target_path);
        target_path = strdup(shell_state->prev_dir);
    }


    DIR* dir = opendir(target_path);
    if (dir == NULL) {
        fprintf(stderr, "No such directory!\n");
        free(target_path);
        return 0;
    }
    free(target_path); // Free the target path as it's no longer needed

    char* entries[4096];
    int count = 0;
    struct dirent* current_entry = readdir(dir);
    while ((current_entry = readdir(dir)) != NULL) {
        if (!show_hidden && current_entry->d_name[0] == '.') {
            continue; // Skip hidden files
        }
        entries[count++] = strdup(current_entry->d_name);
    }

    qsort(entries, count, sizeof(char*), cmp);

    if (!long_format) {
        for (int i = 0; i < count; i++) {
            fprintf(stdout, "%s ", entries[i]);
            free(entries[i]);
        }
        fprintf(stdout, "\n");
    } 
    else {
        for (int i = 0; i < count; i++) {
            fprintf(stdout, "%s\n", entries[i]);
            free(entries[i]);
        }
    }

    closedir(dir);

    return 1;

}

int builtins_log(shell_state_t* shell_state, const arg_node_t* args) {
    args = args->next; // Skip the command name

    if (args == NULL) {
        FILE* history_file = fopen(HISTORY_FILE_PATH, "r");
        if (!history_file) {
            fprintf(stderr, "Failed to open history file");
            return 0;
        }

        char line[4096];
        while (fgets(line, sizeof(line), history_file)) {
            fprintf(stdout, "%s", line);
        }
        fclose(history_file);
    } 
    else if (strcmp(args->arg, "purge") == 0) {
        if (args->next != NULL) {
            fprintf(stderr, "log: Invalid syntax!\n");
            return 0;
        }
        
        FILE* history_file = fopen(HISTORY_FILE_PATH, "w");
        if (!history_file) {
            fprintf(stderr, "Failed to open history file for writing");
            return 0;
        }
        fclose(history_file);
    }
    else if (strcmp(args->arg, "execute") == 0) {
        args = args->next; // Move to the next argument
        if (args == NULL || args->next != NULL) { // Ensure there's exactly one argument after "execute"
            fprintf(stderr, "log: Invalid syntax!\n");
            return 0;
        }
        int execute_idx = atoi(args->arg);
        if (execute_idx <= 0) {
            fprintf(stderr, "log: Invalid syntax!\n");
            return 0;
        }

        FILE* history_file = fopen(HISTORY_FILE_PATH, "r");
        if (!history_file) {
            fprintf(stderr, "Failed to open history file");
            return 0;
        }

        char* history[HISTORY_LIMIT];
        int count = 0;
        char line[4096];
        while (fgets(line, sizeof(line), history_file) && count < HISTORY_LIMIT) {
            history[count++] = strdup(line);
        }
        bool found = false;

        int target_idx = count - execute_idx;
        if (target_idx >= 0 && target_idx < count) {
            found = true;
            strcpy(line, history[target_idx]);
        }

        for (int i = 0; i < count; i++) {
            free(history[i]);
        }
        fclose(history_file);

        if (found) {
            line[strcspn(line, "\r\n")] = '\0'; // Strip trailing CR/LF from fgets
            token_t* token_stream = tokenize_input(line);
            cmd_group_t* shell_cmd = NULL;
            if (token_stream != NULL) {
                shell_cmd = parse_shell_cmd(token_stream);
                free_token_list(token_stream);
            }

            execute_shell_cmd(shell_cmd, shell_state); 
            free_cmd_group(shell_cmd);
        }
        else {
            fprintf(stderr, "log: Invalid syntax!\n");
            return 0;
        }
    }
    else {
        fprintf(stderr, "log: Invalid syntax!\n");
        return 0;
    }

    return 1;
}


