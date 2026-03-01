#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid1, pid2, pid3;
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    
    pid1 = fork();
    if(pid1 == 0)
    {
        printf("Soy el proceso hijo 1, mi PID es %d y el PID de mi padre es %d\n", getpid(), getppid());
    }
    else
    {
        pid2 = fork();
        if(pid2 == 0)
        {
            printf("Soy el proceso hijo 2, mi PID es %d y el PID de mi padre es %d\n", getpid(), getppid());
        }
        else
        {
            pid3 = fork();
            if(pid3 == 0)
            {
                printf("Soy el proceso hijo 3, mi PID es %d y el PID de mi padre es %d\n", getpid(), getppid());
            }
            else
            {
                printf("Soy el proceso padre, mi PID es %d\n", getpid());
            }
        }
    }
    
    wait(NULL); // Esperar a que los procesos hijos terminen
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Tiempo de ejecucion: %f segundos\n", cpu_time_used);
	return 0;
}