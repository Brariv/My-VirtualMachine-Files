#include <stdio.h>
#include <time.h>

int main()
{
    clock_t start, end;
    double cpu_time_used;
    
    start = clock();
    
    for(int i=0; i<1000000; i++)
    {  
        ;
    }

    for(int i=0; i<1000000; i++)
    {  
        ;
    }

    for(int i=0; i<1000000; i++)
    {  
        ;
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Tiempo de ejecucion: %f segundos\n", cpu_time_used);
	return 0;
}
