# Advanced Shell

A feature-rich Unix shell implementation written in C, supporting modern shell features including pipes, redirection, job control, command history, aliases, and more.

## Features

### Core Functionality
- **Command execution** - Run external programs and built-in commands
- **Interactive prompt** - Colorized prompt showing user@hostname:directory
- **Command history** - Persistent history across sessions
- **Tab completion** - Basic command completion support

### Advanced Features
- **Pipes** - Chain commands together: `ls | grep file | wc -l`
- **Redirection** - Input/output redirection: `cmd > file`, `cmd < input`, `cmd >> append`
- **Background jobs** - Run commands in background: `long_command &`
- **Job control** - Manage background jobs with `jobs`, `fg`, `bg`
- **Command chaining** - Conditional execution: `cmd1 && cmd2`, `cmd1 || cmd2`, `cmd1 ; cmd2`
- **Variable expansion** - Use environment variables: `echo $HOME`, `echo ${PATH}`
- **Aliases** - Create command shortcuts: `alias ll='ls -la'`
- **Wildcard expansion** - Glob patterns: `ls *.txt`, `rm file?.log`
- **Script execution** - Run shell scripts from files

### Built-in Commands
- `cd [dir]` - Change directory
- `pwd` - Print working directory  
- `exit [code]` - Exit shell
- `help` - Show help information
- `history` - Display command history
- `jobs` - List active jobs
- `fg [job]` - Bring job to foreground
- `bg [job]` - Send job to background  
- `kill [pid/job]` - Terminate process or job
- `export VAR=value` - Set environment variable
- `unset VAR` - Remove environment variable
- `alias name=value` - Create command alias
- `unalias name` - Remove alias
- `echo [text]` - Display text with variable expansion
- `type command` - Show command type and location

## Installation

### Prerequisites
- GCC compiler
- Make utility
- POSIX-compliant system (Linux, macOS, BSD, WSL)
- Standard C development libraries

### Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/RETCY-unix/Myshell
cd advanced-shell
```

2. Compile the shell:
```bash
make
```

3. Run the shell:
```bash
./shell
```

### Installation (Optional)
To install system-wide:
```bash
sudo make install
```

To uninstall:
```bash
sudo make uninstall
```

## Usage Examples

### Basic Commands
```bash
$ ls -la
$ pwd
$ cd /home/user/documents
$ echo "Hello, World!"
```

### Pipes and Redirection
```bash
$ ls | grep .txt | wc -l
$ cat input.txt | sort > sorted.txt
$ echo "log entry" >> logfile.txt
$ command < input.txt > output.txt
```

### Job Control
```bash
$ long_running_command &
[1] 12345
$ jobs
[1] Running    long_running_command
$ fg 1
$ bg 1
```

### Aliases and Variables
```bash
$ alias ll='ls -la'
$ alias grep='grep --color=always'
$ export EDITOR=vim
$ echo $HOME
$ echo "Path: $PATH"
```

### Command Chaining
```bash
$ make clean && make && ./program
$ test -f file.txt || echo "File not found"
$ command1 ; command2 ; command3
```

## Script Support

The shell can execute script files:

```bash
$ ./shell script.sh
```

Example script (`script.sh`):
```bash
#!/path/to/shell
# This is a comment
echo "Starting script..."
cd /tmp
ls -la
echo "Script completed"
```

## Development

### Build Targets
- `make` - Build the shell
- `make clean` - Remove build artifacts
- `make debug` - Build with debug symbols
- `make release` - Build optimized version
- `make valgrind` - Run with memory leak detection
- `make static-analysis` - Run static code analysis

### Project Structure
```
advanced-shell/
├── shell.h                 # Header file with declarations
├── main.c                  # Main program entry point
├── shell_core.c           # Core shell functionality
├── builtin_commands.c     # Built-in command implementations
├── advanced_features.c    # Pipes, redirection, job control
├── job_control.c          # Background job management
├── history_aliases.c      # History and alias management  
├── utilities.c            # Utility functions
├── Makefile              # Build configuration
└── README.md             # This file
```

## Configuration

The shell supports several environment variables:
- `PS1` - Primary prompt string (default: "$ ")
- `PATH` - Command search path
- `HOME` - User home directory
- `EDITOR` - Default text editor

History is automatically saved to `~/.shell_history`.

## Known Limitations

- Signal handling is basic (Ctrl+C support only)
- No command-line editing features (arrow keys, etc.)
- Limited tab completion
- No brace expansion (`{a,b,c}`)
- No command substitution (`` `command` `` or `$(command)`)
- No here documents (`<<`)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request


## Author

Aymane Ghafour - Initial implementation

## Acknowledgments

- Inspired by bash, zsh, and other Unix shells
- Built using POSIX standards for compatibility
