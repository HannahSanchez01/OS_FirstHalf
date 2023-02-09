#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>


void SIGhandler(int sig)
{
	 /* Signal handling function */
    printf("Control-C was pressed ... exiting\n");
    exit(0);
}


int main(void)
{
    char command[64];
    int c, rc;
   
	 // Signaler
    signal(SIGINT, SIGhandler);

    printf("Execute? ");
    scanf("%64[^\n]", command); //get input; limit length to avoid overflow

	 // clear input buffer
    while (((c=getchar()) != '\n') && (c != EOF)){}

	 rc = fork();

    if(rc==0){ // child process
        	printf("Executing: %s\n", command);
        	system(command);
        	exit(0);
    }

	 else if(rc<0){ // fork failed; exit
	 	fprintf(stderr, "fork failed\n");
		exit(1);
	 }

    else{ // parent process
        wait(NULL);
        printf("Execution complete!\n");
    }

    return 0;
}


