#include "shell.h"

int cmd_cd(char **args) {
    char *dir;
    
    if (args[1] == NULL) {
        dir = get_shell_var("HOME");
        if (!dir) dir = getenv("HOME");
    } else {
        dir = args[1];
    }
    
    if (chdir(dir) != 0) {
        perror("cd");
    } else {
        // Update PWD variable
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            set_shell_var("PWD", cwd);
        }
    }
    
    return 1;
}

int cmd_pwd(char **args) {
    (void)args; // Suppress unused parameter warning
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
    return 1;
}

int cmd_exit(char **args) {
    if (args[1]) {
        int exit_code = atoi(args[1]);
        exit(exit_code);
    }
    return 0;  // Signal to exit main loop
}

int cmd_help(char **args) {
    (void)args; // Suppress unused parameter warning
    printf("Advanced Shell - Built-in commands:\n");
    printf("  cd [dir]          - Change directory\n");
    printf("  pwd               - Print working directory\n");
    printf("  exit [code]       - Exit shell\n");
    printf("  help              - Show this help\n");
    printf("  history           - Show command history\n");
    printf("  jobs              - Show active jobs\n");
    printf("  fg [job]          - Bring job to foreground\n");
    printf("  bg [job]          - Send job to background\n");
    printf("  kill [pid/job]    - Kill process or job\n");
    printf("  export var=value  - Set environment variable\n");
    printf("  unset var         - Unset variable\n");
    printf("  alias name=value  - Create alias\n");
    printf("  unalias name      - Remove alias\n");
    printf("  echo [text]       - Display text\n");
    printf("  type command      - Show command type\n");
    printf("\nFeatures:\n");
    printf("  - Pipes: cmd1 | cmd2\n");
    printf("  - Redirection: cmd > file, cmd < file, cmd >> file\n");
    printf("  - Background: cmd &\n");
    printf("  - Command chaining: cmd1 && cmd2, cmd1 || cmd2, cmd1 ; cmd2\n");
    printf("  - Variable expansion: $VAR\n");
    printf("  - Wildcard expansion: *.txt\n");
    printf("  - Tab completion (basic)\n");
    return 1;
}

int cmd_history(char **args) {
    (void)args; // Suppress unused parameter warning
    for (int i = 0; i < history_count; i++) {
        printf("%4d  %s\n", i + 1, history[i]);
    }
    return 1;
}

int cmd_jobs(char **args) {
    (void)args; // Suppress unused parameter warning
    update_job_status();
    
    job_t *current = job_list;
    while (current) {
        printf("[%d] %s %s\n", 
               current->id,
               (current->status == JOB_RUNNING) ? "Running" :
               (current->status == JOB_STOPPED) ? "Stopped" : "Done",
               current->command);
        current = current->next;
    }
    return 1;
}

int cmd_fg(char **args) {
    int job_id = 1;
    if (args[1]) {
        job_id = atoi(args[1]);
    }
    
    job_t *job = find_job(job_id);
    if (job) {
        if (kill(job->pid, SIGCONT) == 0) {
            job->status = JOB_RUNNING;
            printf("[%d] %s\n", job->id, job->command);
            
            int status;
            waitpid(job->pid, &status, 0);
            remove_job(job->pid);
        } else {
            perror("fg");
        }
    } else {
        printf("Job %d not found\n", job_id);
    }
    return 1;
}

int cmd_bg(char **args) {
    int job_id = 1;
    if (args[1]) {
        job_id = atoi(args[1]);
    }
    
    job_t *job = find_job(job_id);
    if (job) {
        if (kill(job->pid, SIGCONT) == 0) {
            job->status = JOB_RUNNING;
            printf("[%d] %s &\n", job->id, job->command);
        } else {
            perror("bg");
        }
    } else {
        printf("Job %d not found\n", job_id);
    }
    return 1;
}

int cmd_kill(char **args) {
    if (!args[1]) {
        printf("Usage: kill <pid|job>\n");
        return 1;
    }
    
    int pid = atoi(args[1]);
    if (pid == 0 && args[1][0] == '%') {
        // Job specification
        int job_id = atoi(args[1] + 1);
        job_t *job = find_job(job_id);
        if (job) {
            pid = job->pid;
        } else {
            printf("Job %d not found\n", job_id);
            return 1;
        }
    }
    
    if (kill(pid, SIGTERM) == 0) {
        printf("Process %d terminated\n", pid);
        remove_job(pid);
    } else {
        perror("kill");
    }
    return 1;
}

int cmd_export(char **args) {
    if (!args[1]) {
        // Display all environment variables
        char **env = environ;
        while (*env) {
            printf("%s\n", *env);
            env++;
        }
        return 1;
    }
    
    char *equals = strchr(args[1], '=');
    if (equals) {
        *equals = '\0';
        char *name = args[1];
        char *value = equals + 1;
        
        if (setenv(name, value, 1) == 0) {
            set_shell_var(name, value);
        } else {
            perror("export");
        }
    } else {
        printf("Usage: export VAR=value\n");
    }
    return 1;
}

int cmd_unset(char **args) {
    if (!args[1]) {
        printf("Usage: unset VAR\n");
        return 1;
    }
    
    if (unsetenv(args[1]) == 0) {
        unset_shell_var(args[1]);
    } else {
        perror("unset");
    }
    return 1;
}

int cmd_alias(char **args) {
    if (!args[1]) {
        // Display all aliases
        for (int i = 0; i < alias_count; i++) {
            printf("alias %s='%s'\n", aliases[i].name, aliases[i].value);
        }
        return 1;
    }
    
    char *equals = strchr(args[1], '=');
    if (equals) {
        *equals = '\0';
        char *name = args[1];
        char *value = equals + 1;
        add_alias(name, value);
    } else {
        char *value = get_alias(args[1]);
        if (value) {
            printf("alias %s='%s'\n", args[1], value);
        } else {
            printf("alias: %s: not found\n", args[1]);
        }
    }
    return 1;
}

int cmd_unalias(char **args) {
    if (!args[1]) {
        printf("Usage: unalias name\n");
        return 1;
    }
    
    remove_alias(args[1]);
    return 1;
}

int cmd_echo(char **args) {
    for (int i = 1; args[i]; i++) {
        if (i > 1) printf(" ");
        
        // Expand variables
        char *expanded = expand_variables(args[i]);
        printf("%s", expanded);
        if (expanded != args[i]) {
            free(expanded);
        }
    }
    printf("\n");
    return 1;
}

int cmd_type(char **args) {
    if (!args[1]) {
        printf("Usage: type command\n");
        return 1;
    }
    
    // Check if builtin
    char *builtins[] = {"cd", "pwd", "exit", "help", "history", "jobs", 
                       "fg", "bg", "kill", "export", "unset", "alias", 
                       "unalias", "echo", "type", NULL};
    
    for (int i = 0; builtins[i]; i++) {
        if (strcmp(args[1], builtins[i]) == 0) {
            printf("%s is a shell builtin\n", args[1]);
            return 1;
        }
    }
    
    // Check if alias
    char *alias_value = get_alias(args[1]);
    if (alias_value) {
        printf("%s is aliased to '%s'\n", args[1], alias_value);
        return 1;
    }
    
    // Check PATH
    char *path = getenv("PATH");
    if (path) {
        char *path_copy = strdup(path);
        char *dir = strtok(path_copy, ":");
        
        while (dir) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, args[1]);
            
            if (access(full_path, X_OK) == 0) {
                printf("%s is %s\n", args[1], full_path);
                free(path_copy);
                return 1;
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    printf("%s: not found\n", args[1]);
    return 1;
}
