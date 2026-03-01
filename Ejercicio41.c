#include <stdio.h>
#include <unistd.h>


int main()
{
	if(fork()==0)
	{
		//printf("Soy el proceso hijo 1, mi PID es %d y el PID de mi padre es %d\n", getpid(), getppid());
		for (int i = 0; i < 4000000; i++) {
			printf("%d\n", i);
		}
	}
	else
	{
		printf("Soy el proceso padre, mi PID es %d\n", getpid());
		while(true)
        {
            ;
        }
	}
	return 0;
}