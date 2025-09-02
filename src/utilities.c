#include "shell.h"

char *trim_whitespace(char *str) {
    if (!str) return NULL;
    
    // Trim leading whitespace
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;  // All spaces
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end + 1) = '\0';
    
    return str;
}

int is_builtin(char *command) {
    char *builtins[] = {
        "cd", "pwd", "exit", "help", "history", "jobs", 
        "fg", "bg", "kill", "export", "unset", "alias", 
        "unalias", "echo", "type", NULL
    };
    
    for (int i = 0; builtins[i]; i++) {
        if (strcmp(command, builtins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// String utilities
int starts_with(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

int ends_with(const char *str, const char *suffix) {
    int str_len = strlen(str);
    int suffix_len = strlen(suffix);
    
    if (suffix_len > str_len) return 0;
    
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

char *str_replace(const char *orig, const char *rep, const char *with) {
    char *result;
    char *ins;
    char *tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if (!orig || !rep) return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0) return NULL;
    
    if (!with) with = "";
    len_with = strlen(with);

    ins = (char*)orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result) return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}

// File utilities
int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return 0;
    return S_ISDIR(statbuf.st_mode);
}

int is_executable(const char *path) {
    return access(path, X_OK) == 0;
}

// Path utilities
char *get_full_path(const char *command) {
    if (strchr(command, '/')) {
        // Already a path
        return strdup(command);
    }
    
    char *path_env = getenv("PATH");
    if (!path_env) return NULL;
    
    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");
    
    while (dir) {
        char *full_path = malloc(strlen(dir) + strlen(command) + 2);
        sprintf(full_path, "%s/%s", dir, command);
        
        if (is_executable(full_path)) {
            free(path_copy);
            return full_path;
        }
        
        free(full_path);
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    return NULL;
}

// Process utilities
int is_process_running(pid_t pid) {
    return kill(pid, 0) == 0;
}

// Error handling
void shell_error(const char *message) {
    fprintf(stderr, "shell: %s\n", message);
}

void shell_perror(const char *message) {
    perror(message);
}

// Memory management helpers
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        shell_error("memory allocation failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *safe_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        shell_error("memory reallocation failed");
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

char *safe_strdup(const char *s) {
    char *dup = strdup(s);
    if (!dup) {
        shell_error("string duplication failed");
        exit(EXIT_FAILURE);
    }
    return dup;
}

// Array utilities
int count_args(char **args) {
    int count = 0;
    while (args && args[count]) {
        count++;
    }
    return count;
}

char **copy_args(char **args) {
    int count = count_args(args);
    char **copy = malloc((count + 1) * sizeof(char*));
    
    for (int i = 0; i < count; i++) {
        copy[i] = safe_strdup(args[i]);
    }
    copy[count] = NULL;
    
    return copy;
}

// Time utilities
char *get_timestamp(void) {
    time_t now = time(NULL);
    char *timestr = ctime(&now);
    
    // Remove trailing newline
    int len = strlen(timestr);
    if (len > 0 && timestr[len-1] == '\n') {
        timestr[len-1] = '\0';
    }
    
    return timestr;
}
