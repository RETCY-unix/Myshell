#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\n");
        display_prompt();
        fflush(stdout);
    }
}

int main(int argc, char *argv[]) {
    char *input_line = NULL;
    int status = 1;
    
    // Initialize shell
    init_shell();
    
    // Set up signal handling
    signal(SIGINT, signal_handler);
    
    // Check if we're running a script
    if (argc > 1) {
        return run_script(argv[1]);
    }
    
    // Interactive mode
    printf("Advanced Shell v1.0 - Type 'help' for commands\n");
    
    do {
        display_prompt();
        input_line = read_line();
        
        if (input_line == NULL) {
            break;  // EOF (Ctrl+D)
        }
        
        // Add to history if not empty
        if (strlen(input_line) > 0) {
            add_to_history(input_line);
        }
        
        // Handle multi-command input (pipes, &&, ||, ;)
        status = process_complex_command(input_line);
        
        free(input_line);
    } while (status);
    
    cleanup_shell();
    return EXIT_SUCCESS;
}
