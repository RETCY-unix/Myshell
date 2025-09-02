#include "shell.h"

// History functions
void add_to_history(char *line) {
    if (history_count >= MAX_HISTORY) {
        // Remove oldest entry
        free(history[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            history[i] = history[i + 1];
        }
        history_count--;
    }
    
    history[history_count++] = strdup(line);
}

void save_history(void) {
    char *home = get_shell_var("HOME");
    if (!home) home = getenv("HOME");
    if (!home) return;
    
    char history_file[1024];
    snprintf(history_file, sizeof(history_file), "%s/.shell_history", home);
    
    FILE *file = fopen(history_file, "w");
    if (!file) return;
    
    for (int i = 0; i < history_count; i++) {
        fprintf(file, "%s\n", history[i]);
    }
    
    fclose(file);
}

void load_history(void) {
    char *home = get_shell_var("HOME");
    if (!home) home = getenv("HOME");
    if (!home) return;
    
    char history_file[1024];
    snprintf(history_file, sizeof(history_file), "%s/.shell_history", home);
    
    FILE *file = fopen(history_file, "r");
    if (!file) return;
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    while ((read = getline(&line, &len, file)) != -1 && history_count < MAX_HISTORY) {
        // Remove newline
        if (line[read-1] == '\n') {
            line[read-1] = '\0';
        }
        
        if (strlen(line) > 0) {
            history[history_count++] = strdup(line);
        }
    }
    
    free(line);
    fclose(file);
}

// Alias functions
void add_alias(char *name, char *value) {
    // Check if alias already exists
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            free(aliases[i].value);
            aliases[i].value = strdup(value);
            return;
        }
    }
    
    // Add new alias
    if (alias_count < MAX_ALIASES) {
        aliases[alias_count].name = strdup(name);
        aliases[alias_count].value = strdup(value);
        alias_count++;
    }
}

char *get_alias(char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return NULL;
}

void remove_alias(char *name) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            free(aliases[i].name);
            free(aliases[i].value);
            
            // Shift remaining aliases
            for (int j = i; j < alias_count - 1; j++) {
                aliases[j] = aliases[j + 1];
            }
            alias_count--;
            return;
        }
    }
}

// Variable functions
void set_shell_var(char *name, char *value) {
    // Check if variable already exists
    for (int i = 0; i < var_count; i++) {
        if (strcmp(shell_vars[i].name, name) == 0) {
            free(shell_vars[i].value);
            shell_vars[i].value = strdup(value);
            return;
        }
    }
    
    // Add new variable
    if (var_count < MAX_VARS) {
        shell_vars[var_count].name = strdup(name);
        shell_vars[var_count].value = strdup(value);
        var_count++;
    }
}

char *get_shell_var(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(shell_vars[i].name, name) == 0) {
            return shell_vars[i].value;
        }
    }
    return NULL;
}

void unset_shell_var(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(shell_vars[i].name, name) == 0) {
            free(shell_vars[i].name);
            free(shell_vars[i].value);
            
            // Shift remaining variables
            for (int j = i; j < var_count - 1; j++) {
                shell_vars[j] = shell_vars[j + 1];
            }
            var_count--;
            return;
        }
    }
}
