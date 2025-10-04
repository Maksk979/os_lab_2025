#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s --seed N --array_size M --pnum K\n", argv[0]);
        return 1;
    }


    char *new_argv[] = {
        "./sequential_min_max",
        argv[2],  
        argv[4],  
        NULL
    };

    pid_t pid = fork();
    if (pid == 0) {
        execv(new_argv[0], new_argv);
        perror("execv");
        exit(1);
    }

    int status;
    wait(&status);
    printf("Дочерний процесс завершился с кодом %d\n", WEXITSTATUS(status));
    return 0;
}