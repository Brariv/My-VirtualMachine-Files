// Cree un programa en C que ejecute fork() dentro de un ciclo for de cuatro iteraciones.
#include <stdio.h>
#include <unistd.h>


int main()
{
    for(int i=0; i<4; i++)
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
	return 0;
}