#include "../include/prompt.h"
#include "../include/token.h"
#include "../include/command.h"
#include "../include/parser.h"
#include "../include/builtins.h"
#include "../include/execute.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define INPUT_BUFFER_SIZE 4096
char HOME_DIR[PATH_MAX];
char input[PATH_MAX];

int main() {
    init_home_dir();

    shell_state_t shell_state = { .prev_dir = NULL, .prev_dir_valid = false }; 
    
    while (1) {
        show_prompt();

        char* result = fgets(input, sizeof(input), stdin);
        if (result == NULL) {
            if (feof(stdin)) {
                printf("\n");
                break;
            } 
            else {
                perror("Error reading input");
                continue;
            }
        }

        /* Strip trailing CR/LF from fgets */
        input[strcspn(input, "\r\n")] = '\0';

        // If the user just hit Enter, reprint prompt
        if (input[0] == '\0') {
            continue;
        }

        token_t* token_stream = tokenize_input(input);
        // debug_print_tokens(token_stream);

        cmd_group_t* shell_cmd = NULL;
        if (token_stream != NULL) {
            shell_cmd = parse_shell_cmd(token_stream);
            // debug_print_shell_cmd(shell_cmd);
            free_token_list(token_stream);
        }
        else {
            continue;
        }

        execute_shell_cmd(shell_cmd, &shell_state); 
        free_cmd_group(shell_cmd);
    }
    return 0;
}