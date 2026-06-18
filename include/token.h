#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOK_NAME,
    TOK_PIPE,
    TOK_SEMI,
    TOK_AMP,
    TOK_LT,
    TOK_GT,
    TOK_GTGT,
    TOK_EOF
} token_type_t;

typedef struct token {
    token_type_t type;
    char* text;
    struct token* next;
} token_t;

token_type_t get_token_type(char *token);
token_t *create_new_token(char *token);
void free_token_list(token_t *head);
token_t *tokenize_input(char *input);

void debug_print_tokens(token_t *head);

#endif // TOKEN_H