#include "shell.h"

static int next_job_id = 1;

void add_job(pid_t pid, char *command) {
    job_t *new_job = malloc(sizeof(job_t));
    if (!new_job) {
        perror("malloc");
        return;
    }
    
    new_job->id = next_job_id++;
    new_job->pid = pid;
    new_job->command = strdup(command);
    new_job->status = JOB_RUNNING;
    new_job->next = job_list;
    
    job_list = new_job;
}

void remove_job(pid_t pid) {
    job_t **current = &job_list;
    
    while (*current) {
        if ((*current)->pid == pid) {
            job_t *to_remove = *current;
            *current = (*current)->next;
            
            free(to_remove->command);
            free(to_remove);
            return;
        }
        current = &((*current)->next);
    }
}

void update_job_status(void) {
    job_t *current = job_list;
    
    while (current) {
        int status;
        pid_t result = waitpid(current->pid, &status, WNOHANG | WUNTRACED);
        
        if (result > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                current->status = JOB_DONE;
            } else if (WIFSTOPPED(status)) {
                current->status = JOB_STOPPED;
            }
        } else if (result == -1) {
            // Process doesn't exist anymore
            current->status = JOB_DONE;
        }
        
        current = current->next;
    }
    
    // Remove completed jobs
    job_t **current_ptr = &job_list;
    while (*current_ptr) {
        if ((*current_ptr)->status == JOB_DONE) {
            job_t *to_remove = *current_ptr;
            *current_ptr = (*current_ptr)->next;
            
            printf("[%d]+ Done                    %s\n", 
                   to_remove->id, to_remove->command);
            
            free(to_remove->command);
            free(to_remove);
        } else {
            current_ptr = &((*current_ptr)->next);
        }
    }
}

job_t *find_job(int id) {
    job_t *current = job_list;
    
    while (current) {
        if (current->id == id) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

job_t *find_job_by_pid(pid_t pid) {
    job_t *current = job_list;
    
    while (current) {
        if (current->pid == pid) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}
