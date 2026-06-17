#include "parser.h"
#include "token.h"
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

token_t* peek(parser_state_t* state) {
    return state->current_token;
}

token_t* consume(parser_state_t* state) {
    token_t* current = state->current_token;
    if (current != NULL) {
        state->current_token = current->next;
    }

    return current;
}

bool match(parser_state_t* state, token_type_t type) {
    token_t* current = peek(state);
    if (current != NULL && current->type == type) {
        consume(state);
        return true;
    }

    return false;
}

token_t* expect(parser_state_t* state, token_type_t type) {
    token_t* current = peek(state);
    if (current == NULL || current->type != type) {
        return NULL;
    }

    return consume(state);
}

atomic_cmd_t* parse_atomic_cmd(parser_state_t* state) {
    atomic_cmd_t* cmd = create_atomic_cmd();
    if (cmd == NULL) {
        fprintf(stderr, "Failed to create atomic command\n");
        return NULL;
    }

    if (state->current_token == NULL || state->current_token->type != TOK_NAME) {
        fprintf(stderr, "Invalid Syntax : Expected command name\n");
        free_atomic_cmd(cmd);
        return NULL;
    }

    arg_node_t* tail = NULL;
    while (state->current_token != NULL && state->current_token->type != TOK_PIPE && state->current_token->type != TOK_SEMI && state->current_token->type != TOK_AMP && state->current_token->type != TOK_EOF) {
        token_t* token = consume(state);

        if (token->type == TOK_NAME) {
            arg_node_t* arg_node = create_arg_node(token->text);
            if (arg_node == NULL) {
                fprintf(stderr, "Failed to create argument node\n");
                free_atomic_cmd(cmd);
                return NULL;
            }

            if (cmd->args == NULL) {
                cmd->args = arg_node;
                tail = arg_node;
            } 
            else {
                tail->next = arg_node;
                tail = arg_node;
            }
        } 
        else if (token->type == TOK_LT) {
            token_t* filename_token = expect(state, TOK_NAME);
            if (filename_token == NULL) {
                free_atomic_cmd(cmd);
                fprintf(stderr, "Invalid Syntax : Expected filename after '<'\n");
                return NULL;
            }

            free(cmd->input_redir); // Free any existing input redirection
            cmd->input_redir = strdup(filename_token->text);
        } 
        else if (token->type == TOK_GT || token->type == TOK_GTGT) {
            int append = (token->type == TOK_GTGT) ? 1 : 0;
            token_t* filename_token = expect(state, TOK_NAME);
            if (filename_token == NULL) {
                free_atomic_cmd(cmd);
                fprintf(stderr, "Invalid Syntax : Expected filename after '>' or '>>'\n");
                return NULL;
            }

            free(cmd->output_redir); // Free any existing output redirection
            cmd->output_redir = strdup(filename_token->text);
            cmd->append_output = append;
        } 
        else {
            fprintf(stderr, "Invalid Syntax : Unexpected token\n");
            free_atomic_cmd(cmd);
            return NULL;
        }
    }

    return cmd;
}

cmd_group_t* parse_cmd_group(parser_state_t* state) {
    cmd_group_t* group = create_cmd_group();
    if (group == NULL) {
        fprintf(stderr, "Failed to create command group\n");
        return NULL;
    }

    atomic_cmd_t* atomic_cmd = parse_atomic_cmd(state);
    if (atomic_cmd == NULL) {
        free_cmd_group(group);
        return NULL;
    }

    group->atomic_cmds = atomic_cmd;
    atomic_cmd_t* tail = atomic_cmd;

    while (match(state, TOK_PIPE)) {
        atomic_cmd_t* next_atomic_cmd = parse_atomic_cmd(state);
        if (next_atomic_cmd == NULL) {
            free_cmd_group(group);
            return NULL;
        }
        tail->next = next_atomic_cmd;
        tail = next_atomic_cmd;
    }

    if (match(state, TOK_SEMI)) {
        if (state->current_token == NULL ||state->current_token->type != TOK_NAME){
            fprintf(stderr, "Invalid Syntax : Expected command after ';'\n");
            free_cmd_group(group);
            return NULL;
        }

        group->terminator = TERM_SEMI;
    } 
    else if (match(state, TOK_AMP)) {
        group->terminator = TERM_AMP;
    } 
    else if (state->current_token != NULL && state->current_token->type == TOK_EOF) {
        group->terminator = TERM_NONE;
    }
    else {
        fprintf(stderr, "Invalid Syntax : Expected ';', '&', or end of input\n");
        free_cmd_group(group);
        return NULL;
    }

    return group;
}

cmd_group_t* parse_shell_cmd(token_t* token_stream) {
    parser_state_t state = { .current_token = token_stream };
    
    cmd_group_t* head = NULL;
    cmd_group_t* tail = NULL;
    while (state.current_token != NULL && state.current_token->type != TOK_EOF) {
        cmd_group_t* group = parse_cmd_group(&state);
        if (group == NULL) {
            free_cmd_group(head);
            return NULL;
        }

        if (head == NULL) {
            head = group;
            tail = group;
        } 
        else {
            tail->next = group;
            tail = group;
        }
    }
    return head;
}

void debug_print_shell_cmd(cmd_group_t* groups) {
    cmd_group_t* group = groups;
    while (group != NULL) {
        printf("Command Group:\n");
        atomic_cmd_t* cmd = group->atomic_cmds;
        while (cmd != NULL) {
            printf("  Atomic Command: \n");

            // Print args as a comma-separated list with Arfgs = [ , , ]
            arg_node_t* arg = cmd->args;
            printf("    Args = [");
            while (arg != NULL) {
                printf("%s", arg->arg);
                if (arg->next != NULL) {
                    printf(", ");
                }
                arg = arg->next;
            }
            printf("]\n");
            // and print the rest fields of struct also
            if (cmd->input_redir) {
                printf("    Input Redirection: %s\n", cmd->input_redir);
            }
            if (cmd->output_redir) {
                printf("    Output Redirection: %s (append: %d)\n", cmd->output_redir, cmd->append_output);
            }   
            printf("    Append Output: %d\n", cmd->append_output);

            cmd = cmd->next;
        }
        printf("Terminator: %s\n", group->terminator == TERM_SEMI ? ";" : (group->terminator == TERM_AMP ? "&" : "None"));
        group = group->next;
    }
}
