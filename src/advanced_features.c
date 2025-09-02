#include "shell.h"

int process_complex_command(char *line) {
    // Handle empty line
    if (strlen(trim_whitespace(line)) == 0) {
        return 1;
    }
    
    // Expand variables first
    char *expanded_line = expand_variables(line);
    
    // Check for background process
    int background = 0;
    char *line_copy = strdup(expanded_line);
    int len = strlen(line_copy);
    if (len > 0 && line_copy[len-1] == '&') {
        background = 1;
        line_copy[len-1] = '\0';
        line_copy = trim_whitespace(line_copy);
    }
    
    // Split by command separators (&&, ||, ;)
    char *commands[MAX_ARGS];
    int cmd_count = 0;
    char *separators[MAX_ARGS];
    int sep_count = 0;
    
    char *current = line_copy;
    char *start = current;
    
    while (*current) {
        if (strncmp(current, "&&", 2) == 0) {
            *current = '\0';
            commands[cmd_count++] = trim_whitespace(start);
            separators[sep_count++] = "&&";
            current += 2;
            start = current;
        } else if (strncmp(current, "||", 2) == 0) {
            *current = '\0';
            commands[cmd_count++] = trim_whitespace(start);
            separators[sep_count++] = "||";
            current += 2;
            start = current;
        } else if (*current == ';') {
            *current = '\0';
            commands[cmd_count++] = trim_whitespace(start);
            separators[sep_count++] = ";";
            current++;
            start = current;
        } else {
            current++;
        }
    }
    commands[cmd_count++] = trim_whitespace(start);
    
    // Execute command chain
    int last_status = 1;
    for (int i = 0; i < cmd_count; i++) {
        if (i > 0) {
            // Check separator condition
            if (strcmp(separators[i-1], "&&") == 0 && last_status == 0) {
                break;  // Previous command failed
            } else if (strcmp(separators[i-1], "||") == 0 && last_status != 0) {
                break;  // Previous command succeeded
            }
        }
        
        // Check for pipes in this command
        if (strchr(commands[i], '|')) {
            char *pipe_commands[MAX_ARGS];
            int pipe_count = 0;
            
            char *cmd_copy = strdup(commands[i]);
            char *token = strtok(cmd_copy, "|");
            while (token && pipe_count < MAX_ARGS - 1) {
                pipe_commands[pipe_count++] = trim_whitespace(token);
                token = strtok(NULL, "|");
            }
            
            last_status = handle_pipes(pipe_commands, pipe_count);
            free(cmd_copy);
        } else {
            // Single command, check for redirection
            char *input_file = NULL;
            char *output_file = NULL;
            int append = 0;
            
            char *cmd_copy = strdup(commands[i]);
            char **args = split_line(cmd_copy);
            
            // Parse redirection
            int arg_count = 0;
            while (args[arg_count]) {
                if (strcmp(args[arg_count], "<") == 0 && args[arg_count + 1]) {
                    input_file = args[arg_count + 1];
                    // Remove redirection from args
                    free(args[arg_count]);
                    free(args[arg_count + 1]);
                    args[arg_count] = NULL;
                    break;
                } else if (strcmp(args[arg_count], ">") == 0 && args[arg_count + 1]) {
                    output_file = args[arg_count + 1];
                    append = 0;
                    free(args[arg_count]);
                    free(args[arg_count + 1]);
                    args[arg_count] = NULL;
                    break;
                } else if (strcmp(args[arg_count], ">>") == 0 && args[arg_count + 1]) {
                    output_file = args[arg_count + 1];
                    append = 1;
                    free(args[arg_count]);
                    free(args[arg_count + 1]);
                    args[arg_count] = NULL;
                    break;
                }
                arg_count++;
            }
            
            if (background) {
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process
                    if (input_file || output_file) {
                        handle_redirection(args, input_file, output_file, append);
                        exit(EXIT_SUCCESS);
                    } else {
                        if (execvp(args[0], args) == -1) {
                            printf("%s: command not found\n", args[0]);
                            exit(EXIT_FAILURE);
                        }
                    }
                } else if (pid > 0) {
                    // Parent process
                    add_job(pid, commands[i]);
                    printf("[%d] %d\n", 1, pid);
                } else {
                    perror("fork");
                }
            } else {
                // Foreground execution
                if (input_file || output_file) {
                    last_status = handle_redirection(args, input_file, output_file, append);
                } else {
                    last_status = execute_command(args);
                }
            }
            
            free(cmd_copy);
            free_args(args);
        }
    }
    
    free(line_copy);
    if (expanded_line != line) {
        free(expanded_line);
    }
    
    return last_status;
}

int handle_pipes(char **commands, int num_commands) {
    if (num_commands <= 1) {
        char **args = split_line(commands[0]);
        int status = execute_command(args);
        free_args(args);
        return status;
    }
    
    int pipes[num_commands - 1][2];
    pid_t pids[num_commands];
    
    // Create all pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 0;
        }
    }
    
    // Create processes for each command
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("fork");
            return 0;
        } else if (pids[i] == 0) {
            // Child process
            
            // Set up input redirection (except for first command)
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Set up output redirection (except for last command)
            if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute command
            char **args = split_line(commands[i]);
            if (execvp(args[0], args) == -1) {
                printf("%s: command not found\n", args[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    // Parent process: close all pipes and wait for children
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all processes
    int status = 1;
    for (int i = 0; i < num_commands; i++) {
        int child_status;
        waitpid(pids[i], &child_status, 0);
        if (i == num_commands - 1) {  // Status of last command
            status = WEXITSTATUS(child_status) == 0 ? 1 : 0;
        }
    }
    
    return status;
}

int handle_redirection(char **args, char *input_file, char *output_file, int append) {
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        return 0;
    } else if (pid == 0) {
        // Child process
        
        // Handle input redirection
        if (input_file) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in == -1) {
                perror("open input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        
        // Handle output redirection
        if (output_file) {
            int flags = O_WRONLY | O_CREAT;
            if (append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            
            int fd_out = open(output_file, flags, 0644);
            if (fd_out == -1) {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        // Execute command
        if (execvp(args[0], args) == -1) {
            printf("%s: command not found\n", args[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status) == 0 ? 1 : 0;
    }
    
    return 1;
}

char *expand_variables(char *str) {
    if (!str || strchr(str, '$') == NULL) {
        return str;  // No variables to expand
    }
    
    char *result = malloc(strlen(str) * 2);  // Allocate extra space
    char *dst = result;
    char *src = str;
    
    while (*src) {
        if (*src == '$' && (*(src+1) == '{' || isalnum(*(src+1)) || *(src+1) == '_')) {
            src++;  // Skip $
            
            char var_name[256];
            int var_len = 0;
            
            if (*src == '{') {
                src++;  // Skip {
                while (*src && *src != '}' && var_len < 255) {
                    var_name[var_len++] = *src++;
                }
                if (*src == '}') src++;  // Skip }
            } else {
                while (*src && (isalnum(*src) || *src == '_') && var_len < 255) {
                    var_name[var_len++] = *src++;
                }
            }
            
            var_name[var_len] = '\0';
            
            // Get variable value
            char *var_value = get_shell_var(var_name);
            if (!var_value) var_value = getenv(var_name);
            if (!var_value) var_value = "";
            
            // Copy variable value to result
            while (*var_value) {
                *dst++ = *var_value++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
    return result;
}

char *expand_wildcards(char *pattern) {
    glob_t glob_result;
    
    if (glob(pattern, GLOB_TILDE, NULL, &glob_result) == 0) {
        // Build result string from matches
        size_t total_len = 0;
        for (size_t i = 0; i < glob_result.gl_pathc; i++) {
            total_len += strlen(glob_result.gl_pathv[i]) + 1;
        }
        
        char *result = malloc(total_len + 1);
        result[0] = '\0';
        
        for (size_t i = 0; i < glob_result.gl_pathc; i++) {
            if (i > 0) strcat(result, " ");
            strcat(result, glob_result.gl_pathv[i]);
        }
        
        globfree(&glob_result);
        return result;
    } else {
        return strdup(pattern);  // No matches, return original
    }
}

int run_script(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("script");
        return EXIT_FAILURE;
    }
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int status = 1;
    
    while ((read = getline(&line, &len, file)) != -1) {
        // Remove newline
        if (line[read-1] == '\n') {
            line[read-1] = '\0';
        }
        
        // Skip empty lines and comments
        char *trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0 || trimmed[0] == '#') {
            continue;
        }
        
        // Execute line
        status = process_complex_command(trimmed);
        if (!status) break;  // Exit command
    }
    
    free(line);
    fclose(file);
    return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
