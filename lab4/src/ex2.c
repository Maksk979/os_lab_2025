#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        printf("Дочерний процесс (PID %d) завершается...\n", getpid());
        exit(0);
    } else {
        printf("Родитель (PID %d) создал дочерний процесс (PID %d)\n", getpid(), pid);
        printf("Дочерний процесс стал зомби.\n");
        printf("Ожидаем 30 секунд... (зомби процесс существует в течение этого времени)\n");

        sleep(30); 

        wait(NULL);
        printf("Зомби процесс завершился.\n");
    }

    return 0;
}