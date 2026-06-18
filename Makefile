CC = gcc

CFLAGS = -std=c99 \
          -D_POSIX_C_SOURCE=200809L \
          -D_XOPEN_SOURCE=700 \
          -Wall -Wextra -Werror \
          -Wno-unused-parameter \
          -fno-asm

all:
	$(CC) $(CFLAGS) src/shell.c src/prompt.c src/token.c src/command.c src/parser.c src/builtins.c src/execute.c -o shell.out

clean:
	rm -f shell.out