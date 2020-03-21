#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>

void find_dir(char *path){

    if (path == NULL) return;
    DIR* dir = opendir(path);
    if (dir == NULL){
        printf("Cannot open directory\n");
        exit(EXIT_FAILURE);
    }

    struct stat buffer;
    lstat(path, &buffer);
    if (S_ISDIR(buffer.st_mode)){
        pid_t pid_fork = fork();

        if (pid_fork < 0){
            printf("Cannot fork\n");
            exit(EXIT_FAILURE);
        }

        else if(pid_fork == 0){
            
            // get process ID (PID) of the calling process
            printf("DIRECTORY: %s, PID: %d\n", path, getpid());
            
            // replace the current process image with a new process image
            int exec_status = execlp("ls", "ls", "-l", path, NULL);
            if (exec_status != 0){
                printf("Exec failed");
                exit(EXIT_FAILURE);
            }
            exit(exec_status);
        }

        else{
            wait(0);
        }
    }
    
    
    struct dirent *file;
    char new_path[256];
    while ((file = readdir(dir)) != NULL){
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, file -> d_name);

        // check information about a file
        if (lstat(new_path, &buffer) < 0){
            printf("Cannot lstat file %s: ", new_path);
            exit(EXIT_FAILURE);
        }
        
        // check if file is a directory
        if (S_ISDIR(buffer.st_mode)){

            if (strcmp(file -> d_name, ".") == 0 || strcmp(file -> d_name, "..") == 0){
                continue;
            }
            find_dir(new_path);
        }
    }
    closedir(dir);
}

int main(int argc, char** argv){
    if (argc != 2){
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    char* path = argv[1];

    find_dir(path);

    return 0;
}