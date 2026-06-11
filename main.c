#include "prompt.h"
#include <stdio.h>

int main() {
    init_home_dir();
    
    while (1) {
        print_prompt();
        read_input();
    }
    return 0;
}