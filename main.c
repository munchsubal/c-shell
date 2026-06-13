#include "prompt.h"
#include "token.h"
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

        /* If the user just hit Enter, reprint prompt */
        if (input[0] == '\0') {
            continue;
        }

        token_t* tokens = tokenize_input(input);
        debug_print_tokens(tokens);
    }
    return 0;
}