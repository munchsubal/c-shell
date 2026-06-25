# C Shell

A Unix-like shell implemented in **C** using POSIX system calls. The project focuses on process creation, command parsing, built-in utilities, persistent command history, and file descriptor manipulation through I/O redirection.

---

## Features

### Interactive Shell
- Custom shell prompt displaying:
  - Username
  - Hostname
  - Current working directory
- Displays the home directory using `~`
- Interactive command execution loop

### Command Parsing
- Recursive-descent parser based on a context-free grammar
- Handles arbitrary whitespace between tokens
- Syntax validation with descriptive error messages
- Supports parsing of:
  - Commands and arguments
  - Input redirection (`<`)
  - Output redirection (`>`, `>>`)
  - Sequential (`;`) and background (`&`) operators (parsing support)

### Built-in Commands

#### `hop`
A custom implementation of `cd`.

Supports:
- Home directory (`~`)
- Current directory (`.`)
- Parent directory (`..`)
- Previous working directory (`-`)
- Relative and absolute paths

---

#### `reveal`
Directory listing utility similar to `ls`.

Supports:
- Hidden files (`-a`)
- Single-line output (`-l`)
- Combined flags
- Lexicographic sorting

---

#### `log`
Persistent shell history.

Features:
- Stores the last 15 commands
- History persists across shell sessions
- Consecutive duplicate commands are ignored
- Execute previous commands
- Clear command history

---

## External Command Execution

- Executes arbitrary programs using POSIX process creation.
- Uses:
  - `fork()`
  - `execvp()`
  - `waitpid()`
- Gracefully handles invalid commands.

Example:

```bash
echo Hello
cat file.txt
sleep 2
grep main shell.c
```

---

## File Redirection

### Input Redirection (`<`)

Redirects standard input from a file.

Implemented using:

- `open()`
- `dup2()`
- File descriptors

Example:

```bash
cat < input.txt
wc < input.txt
```

---

### Output Redirection (`>`)

Redirects standard output to a file.

Features:

- Creates file if it does not exist
- Overwrites existing contents

Example:

```bash
echo Hello > output.txt
```

---

### Output Append (`>>`)

Appends output to an existing file.

Example:

```bash
echo Hello >> output.txt
```

---

## Project Structure

```text
.
├── include/
│   ├── builtins.h
│   ├── command.h
│   ├── execute.h
│   ├── parser.h
│   ├── prompt.h
│   ├── shell.h
│   └── token.h
│
├── src/
│   ├── builtins.c
│   ├── command.c
│   ├── execute.c
│   ├── parser.c
│   ├── prompt.c
│   ├── shell.c
│   └── token.c
│
├── .gitignore
├── history.txt
├── Makefile
├── README.md
└── shell.out
```

### Directory Overview

- **include/** – Header files containing data structures, parser definitions, built-in command interfaces, and shell declarations.
- **src/** – Source files implementing tokenization, parsing, command execution, prompt handling, built-in commands, and shell logic.
- **history.txt** – Persistent command history used by the `log` builtin.
- **Makefile** – Build configuration for compiling the shell.
- **README.md** – Project documentation.

---

## Execution Flow

```
User Input
      │
      ▼
   Tokenizer
      │
      ▼
     Parser
      │
      ▼
 Command Structure
      │
      ▼
 Built-in / External Command
      │
      ▼
      fork()
      │
      ▼
 File Redirection (dup2)
      │
      ▼
     execvp()
```

---

## POSIX System Calls Used

- `fork()`
- `execvp()`
- `waitpid()`
- `open()`
- `dup2()`
- `close()`
- `chdir()`
- `getcwd()`
- `gethostname()`
- `getlogin_r()`

---

## Build

```bash
make all
```

Run:

```bash
./shell.out
```

---

## Example

```bash
<user@system:~> reveal -l
include
src
README.md

<user@system:~> hop ..

<user@system:/home/user> echo Hello > file.txt

<user@system:/home/user> cat < file.txt
Hello
```

---

## Future Enhancements

- Command pipelines (`|`)
- Sequential execution (`;`)
- Background execution (`&`)
- Job control
- Signal handling
- Process management (`fg`, `bg`, `activities`)