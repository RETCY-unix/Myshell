#include "shell.h"

// Global variables
job_t *job_list = NULL;
alias_t aliases[MAX_ALIASES];
shell_var_t shell_vars[MAX_VARS];
char *history[MAX_HISTORY];
int history_count = 0;
int alias_count = 0;
int var_count = 0;

void init_shell(void) {
    // Initialize variables
    set_shell_var("PS1", "$ ");
    char *path = getenv("PATH");
    if (path) set_shell_var("PATH", path);
    char *home = getenv("HOME");
    if (home) set_shell_var("HOME", home);
    char *user = getenv("USER");
    if (user) set_shell_var("USER", user);
    
    // Load history
    load_history();
    
    // Set up job control
    setpgid(0, 0);
}

void cleanup_shell(void) {
    save_history();
    
    // Free history
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    
    // Free aliases
    for (int i = 0; i < alias_count; i++) {
        free(aliases[i].name);
        free(aliases[i].value);
    }
    
    // Free variables
    for (int i = 0; i < var_count; i++) {
        free(shell_vars[i].name);
        free(shell_vars[i].value);
    }
    
    // Free job list
    job_t *current = job_list;
    while (current) {
        job_t *next = current->next;
        free(current->command);
        free(current);
        current = next;
    }
}

char *read_line(void) {
    char *line = NULL;
    size_t bufsize = 0;
    
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            return NULL;  // EOF
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    
    // Remove trailing newline
    size_t len = strlen(line);
    if (len > 0 && line[len-1] == '\n') {
        line[len-1] = '\0';
    }
    
    return line;
}

char **split_line(char *line) {
    int bufsize = MAX_ARGS;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    
    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        tokens[position] = strdup(token);
        position++;
        
        if (position >= bufsize) {
            bufsize += MAX_ARGS;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

void display_prompt(void) {
    char cwd[1024];
    char *ps1 = get_shell_var("PS1");
    char *user = get_shell_var("USER");
    char hostname[256];
    
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");
    }
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "unknown");
    }
    
    // Simple prompt with colors
    printf("%s%s@%s%s:%s%s%s %s", 
           COLOR_GREEN, user ? user : "user",
           hostname,
           COLOR_RESET,
           COLOR_BLUE, cwd,
           COLOR_RESET,
           ps1 ? ps1 : "$ ");
}

int execute_command(char **args) {
    if (args[0] == NULL) {
        return 1;  // Empty command
    }
    
    // Check for alias
    char *alias_value = get_alias(args[0]);
    if (alias_value) {
        // Replace first argument with alias value
        char *expanded = malloc(strlen(alias_value) + 1);
        strcpy(expanded, alias_value);
        
        // Split alias and prepend to args
        char **alias_args = split_line(expanded);
        
        // Create new args array with alias expanded
        int alias_count = 0;
        while (alias_args[alias_count]) alias_count++;
        
        int original_count = 0;
        while (args[original_count]) original_count++;
        
        char **new_args = malloc((alias_count + original_count) * sizeof(char*));
        
        // Copy alias args
        for (int i = 0; i < alias_count; i++) {
            new_args[i] = alias_args[i];
        }
        
        // Copy remaining original args (skip first)
        for (int i = 1; i < original_count; i++) {
            new_args[alias_count + i - 1] = args[i];
        }
        new_args[alias_count + original_count - 1] = NULL;
        
        free(expanded);
        free(alias_args);
        args = new_args;
    }
    
    // Built-in commands
    if (strcmp(args[0], "cd") == 0) return cmd_cd(args);
    if (strcmp(args[0], "pwd") == 0) return cmd_pwd(args);
    if (strcmp(args[0], "exit") == 0) return cmd_exit(args);
    if (strcmp(args[0], "help") == 0) return cmd_help(args);
    if (strcmp(args[0], "history") == 0) return cmd_history(args);
    if (strcmp(args[0], "jobs") == 0) return cmd_jobs(args);
    if (strcmp(args[0], "fg") == 0) return cmd_fg(args);
    if (strcmp(args[0], "bg") == 0) return cmd_bg(args);
    if (strcmp(args[0], "kill") == 0) return cmd_kill(args);
    if (strcmp(args[0], "export") == 0) return cmd_export(args);
    if (strcmp(args[0], "unset") == 0) return cmd_unset(args);
    if (strcmp(args[0], "alias") == 0) return cmd_alias(args);
    if (strcmp(args[0], "unalias") == 0) return cmd_unalias(args);
    if (strcmp(args[0], "echo") == 0) return cmd_echo(args);
    if (strcmp(args[0], "type") == 0) return cmd_type(args);
    
    // External command
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            printf("%s: command not found\n", args[0]);
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
    
    return 1;
}

void free_args(char **args) {
    if (args) {
        int i = 0;
        while (args[i]) {
            free(args[i]);
            i++;
        }
        free(args);
    }
}
