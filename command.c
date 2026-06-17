#include "command.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

cmd_group_t* init_shell_cmd() {
    cmd_group_t* head = (cmd_group_t*)malloc(sizeof(cmd_group_t));
    if (!head) {
        perror("Failed to allocate memory for command group");
        return NULL;
    }

    head->atomic_cmds = NULL;
    head->terminator = TERM_NONE;
    head->next = NULL;
    return head;
}

cmd_group_t* create_cmd_group() {
    cmd_group_t* group = (cmd_group_t*)malloc(sizeof(cmd_group_t));
    if (!group) {
        perror("Failed to allocate memory for command group");
        return NULL;
    }

    group->atomic_cmds = NULL;
    group->terminator = TERM_NONE;
    group->next = NULL;
    return group;
}

atomic_cmd_t* create_atomic_cmd() {
    atomic_cmd_t* atomic_cmd = (atomic_cmd_t*)malloc(sizeof(atomic_cmd_t));
    if (!atomic_cmd) {
        perror("Failed to allocate memory for atomic command");
        return NULL;
    }

    atomic_cmd->args = NULL;
    atomic_cmd->input_redir = NULL;
    atomic_cmd->output_redir = NULL;
    atomic_cmd->append_output = 0;
    atomic_cmd->next = NULL;
    return atomic_cmd;
}

arg_node_t* create_arg_node(char* arg) {
    arg_node_t* arg_node = (arg_node_t*)malloc(sizeof(arg_node_t));
    if (!arg_node) {
        perror("Failed to allocate memory for argument node");
        return NULL;
    }

    arg_node->arg = strdup(arg);
    arg_node->next = NULL;
    return arg_node;
}

void set_input_redirection(atomic_cmd_t* cmd, char* filename) {
    cmd->input_redir = strdup(filename);
}

void set_output_redirection(atomic_cmd_t* cmd, char* filename, int append) {
    cmd->output_redir = strdup(filename);
    cmd->append_output = append;
}

void free_arg_list(arg_node_t* head) {
    arg_node_t* curr = head;
    while (curr != NULL) {
        arg_node_t* next = curr->next;
        free(curr->arg);
        free(curr);
        curr = next;
    }
}

void free_atomic_cmd(atomic_cmd_t* cmd) {
    if (cmd == NULL) return;

    free_arg_list(cmd->args);
    free(cmd->input_redir);
    free(cmd->output_redir);
    free(cmd);
}

void free_cmd_group(cmd_group_t* group) {
    if (group == NULL) return;

    atomic_cmd_t* curr_atomic = group->atomic_cmds;
    while (curr_atomic != NULL) {
        atomic_cmd_t* next_atomic = curr_atomic->next;
        free_atomic_cmd(curr_atomic);
        curr_atomic = next_atomic;
    }
    free(group);
}