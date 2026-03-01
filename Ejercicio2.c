#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main()
{
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    
    for(int i=0; i<3; i++)
    {  
        if(fork()==0)
        {
            printf("Soy el proceso hijo %d, mi PID es %d y el PID de mi padre es %d\n", i+1, getpid(), getppid());
        }
        else
        {
            printf("Soy el proceso padre, mi PID es %d\n", getpid());
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Tiempo de ejecucion: %f segundos\n", cpu_time_used);
	return 0;

}
