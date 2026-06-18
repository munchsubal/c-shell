#include "../include/token.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

token_type_t get_token_type(char* token) {
    if (strcmp(token, "|") == 0) {
        return TOK_PIPE;
    } 
    else if (strcmp(token, ";") == 0) {
        return TOK_SEMI;
    } 
    else if (strcmp(token, "&") == 0) {
        return TOK_AMP;
    } 
    else if (strcmp(token, "<") == 0) {
        return TOK_LT;
    } 
    else if (strcmp(token, ">") == 0) {
        return TOK_GT;
    } 
    else if (strcmp(token, ">>") == 0) {
        return TOK_GTGT;
    } 
    else if (strcmp(token, "EOF") == 0) {
        return TOK_EOF;
    }
    return TOK_NAME;
}

token_t* create_new_token(char* text) {
    token_t* token_node = (token_t*)malloc(sizeof(token_t));
    token_node->type = get_token_type(text);
    token_node->text = strdup(text);
    token_node->next = NULL;

    return token_node;
}

void free_token_list(token_t* head) {
    token_t* curr = head;
    while (curr != NULL) {
        token_t* next = curr->next;
        free(curr->text);
        free(curr);
        curr = next;
    }
}

token_t* tokenize_input(char* input) {
    token_t* head = NULL;
    token_t* tail = NULL;

    int idx = 0;
    token_t* new_token = NULL;
    while (input[idx] != '\0') {
        if (isspace(input[idx])) {
            idx++;
            continue;
        }
        else {
            if (input[idx] == '<') {
                new_token = create_new_token("<");
                idx++;
            }
            else if (input[idx] == '>') {
                if (input[idx + 1] == '>') {
                    new_token = create_new_token(">>");
                    idx += 2;
                } 
                else {
                    new_token = create_new_token(">");
                    idx++;
                }
            }
            else if (input[idx] == '|') {
                new_token = create_new_token("|");
                idx++;
            }
            else if (input[idx] == ';') {
                new_token = create_new_token(";");
                idx++;
            }
            else if (input[idx] == '&') {
                new_token = create_new_token("&");
                idx++;
            }
            else {
                int start = idx;
                while (input[idx] != '\0' && !isspace(input[idx]) && input[idx] != '<' && input[idx] != '>' && input[idx] != '|' && input[idx] != ';' && input[idx] != '&') {
                    idx++;
                }
                char* token_text = strndup(input + start, idx - start);
                new_token = create_new_token(token_text);
                free(token_text);
            }
        }

        if (head == NULL) {
            head = new_token;
            tail = new_token;
        } 
        else {
            tail->next = new_token;
            tail = new_token;
        }
    }

    if (head == NULL) {
        head = create_new_token("EOF");
    } 
    else {
        tail->next = create_new_token("EOF");
        tail = tail->next;
    }
    return head;
}

void debug_print_tokens(token_t* head) {
    token_t* curr = head;
    while (curr != NULL) {
        printf("Token: %s, Type: %d\n", curr->text, curr->type);
        curr = curr->next;
    }
}

