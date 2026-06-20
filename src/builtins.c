#include "../include/builtins.h"
#include "../include/command.h"
#include "../include/shell.h"
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>

extern char HOME_DIR[PATH_MAX];

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
            printf("%s ", entries[i]);
            free(entries[i]);
        }
        printf("\n");
    } 
    else {
        for (int i = 0; i < count; i++) {
            printf("%s\n", entries[i]);
            free(entries[i]);
        }
    }

    closedir(dir);

    return 1;

}


