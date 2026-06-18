#ifndef COMMAND_H
#define COMMAND_H

typedef enum {
    TERM_NONE,
    TERM_SEMI,
    TERM_AMP
} term_t;

typedef struct arg_node_t {
    char* arg; // argument string
    struct arg_node_t* next; // next argument in the list
} arg_node_t;

typedef struct atomic_cmd_t {
    arg_node_t* args; // linked list of arguments, first one is the command name
    char* input_redir;  // NULL if no redirection, otherwise the filename
    char* output_redir; // NULL if no redirection, otherwise the filename
    int append_output; // 1 if >>, 0 if >
    struct atomic_cmd_t* next; // next atomic command in a cmd_group (for pipelines)
} atomic_cmd_t;

typedef struct cmd_group_t {
    atomic_cmd_t* atomic_cmds; // linked list of atomic commands in the group
    term_t terminator; // how this command group is terminated (none, ;, or &)
    struct cmd_group_t* next; // for multiple command groups in a single line
} cmd_group_t;

cmd_group_t* init_shell_cmd(); // Command groups List initialization for the shell
cmd_group_t* create_cmd_group();
atomic_cmd_t* create_atomic_cmd();
arg_node_t* create_arg_node(char* arg);
void set_input_redirection(atomic_cmd_t* cmd, char* filename);
void set_output_redirection(atomic_cmd_t* cmd, char* filename, int append);
void free_arg_list(arg_node_t* head);
void free_atomic_cmd(atomic_cmd_t* cmd);
void free_cmd_group(cmd_group_t* group);

#endif // COMMAND_H