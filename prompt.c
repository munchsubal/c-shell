#include "prompt.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

extern char HOME_DIR[PATH_MAX];

void init_home_dir() {
    getcwd(HOME_DIR, sizeof(HOME_DIR));
}

void show_prompt() {
    char username[1024];
    getlogin_r(username, sizeof(username));

    char hostname[1024];
    gethostname(hostname, sizeof(hostname));

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    //replace HOME with tilde
    if (strncmp(cwd, HOME_DIR, strlen(HOME_DIR)) == 0 && (cwd[strlen(HOME_DIR)] == '/' || cwd[strlen(HOME_DIR)] == '\0')) {
        printf("<%s@%s:~%s> ", username, hostname, cwd + strlen(HOME_DIR));
    } 
    else {
        printf("<%s@%s:%s> ", username, hostname, cwd);
    }
    fflush(stdout);
}
