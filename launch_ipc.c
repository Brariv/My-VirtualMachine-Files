#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execl("./ipc", "./ipc", "3", "a", NULL);
        perror("execl child");
        exit(1);
    } else {
        usleep(200000); /* pequeña pausa para que sea más estable */
        execl("./ipc", "./ipc", "2", "b", NULL);
        perror("execl parent");
        exit(1);
    }
}