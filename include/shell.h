#ifndef SHELL_H
#define SHELL_H

// Feature test macros - must come first
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <time.h>
#include <glob.h>
#include <signal.h>
#include <termios.h>
#include <ctype.h>

// Constants
#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_HISTORY 1000
#define MAX_JOBS 100
#define MAX_ALIASES 100
#define MAX_VARS 100

// Color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

// Job status
typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} job_status_t;

// Job structure
typedef struct job {
    int id;
    pid_t pid;
    char *command;
    job_status_t status;
    struct job *next;
} job_t;

// Alias structure
typedef struct alias {
    char *name;
    char *value;
} alias_t;

// Environment variable structure
typedef struct shell_var {
    char *name;
    char *value;
} shell_var_t;

// Global variables
extern char **environ;
extern job_t *job_list;
extern alias_t aliases[MAX_ALIASES];
extern shell_var_t shell_vars[MAX_VARS];
extern char *history[MAX_HISTORY];
extern int history_count;
extern int alias_count;
extern int var_count;

// Core functions
void init_shell(void);
void cleanup_shell(void);
char *read_line(void);
char **split_line(char *line);
int execute_command(char **args);
void display_prompt(void);

// Built-in commands
int cmd_cd(char **args);
int cmd_pwd(char **args);
int cmd_exit(char **args);
int cmd_help(char **args);
int cmd_history(char **args);
int cmd_jobs(char **args);
int cmd_fg(char **args);
int cmd_bg(char **args);
int cmd_kill(char **args);
int cmd_export(char **args);
int cmd_unset(char **args);
int cmd_alias(char **args);
int cmd_unalias(char **args);
int cmd_echo(char **args);
int cmd_type(char **args);

// Advanced features
int process_complex_command(char *line);
int handle_pipes(char **commands, int num_commands);
int handle_redirection(char **args, char *input_file, char *output_file, int append);
char *expand_wildcards(char *pattern);
char *expand_variables(char *str);
int run_script(char *filename);

// Job control
void add_job(pid_t pid, char *command);
void remove_job(pid_t pid);
void update_job_status(void);
job_t *find_job(int id);
job_t *find_job_by_pid(pid_t pid);

// History
void add_to_history(char *line);
void save_history(void);
void load_history(void);

// Aliases
void add_alias(char *name, char *value);
char *get_alias(char *name);
void remove_alias(char *name);

// Variables
void set_shell_var(char *name, char *value);
char *get_shell_var(char *name);
void unset_shell_var(char *name);

// Utilities
char *trim_whitespace(char *str);
int is_builtin(char *command);
void free_args(char **args);

#endif
