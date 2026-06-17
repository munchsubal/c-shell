#include "prompt.h"
#include "token.h"
#include "command.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>

#define HOME_DIR_SIZE 1024
#define INPUT_BUFFER_SIZE 1024

char HOME_DIR[HOME_DIR_SIZE];
char input[INPUT_BUFFER_SIZE];

int main() {
    init_home_dir();
    
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
            debug_print_shell_cmd(shell_cmd);
            free_token_list(token_stream);
        }
        else {
            continue;
        }


        // if (shell_cmd == NULL) {
        //     fprintf(stderr, "Failed to parse command\n");
        // } 
        // else {
        //     // For now, just print the parsed command structure
        //     // In a full implementation, we would execute the commands here
        //     printf("Parsed command successfully\n");
        // }
    }
    return 0;
}