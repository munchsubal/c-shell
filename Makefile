CC = gcc

CFLAGS = -std=c99 \
          -D_POSIX_C_SOURCE=200809L \
          -D_XOPEN_SOURCE=700 \
          -Wall -Wextra -Werror \
          -Wno-unused-parameter \
          -fno-asm

all:
	$(CC) $(CFLAGS) shell.c prompt.c token.c command.c parser.c builtins.c execute.c -o shell.out

clean:
	rm -f shell.out