#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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

	 struct sigaction sigSetValue;

	 int max = 64;
	 char *command;
    char buf[max];
    int c, rc;
   
	 // Set up sigactions

	 sigaction( , &sigSetValue, NULL);
	 sigaction( , &sigSetValue, NULL);
	 sigaction( , &sigSetValue, NULL);
	 sigaction( , &sigSetValue, NULL);
	 sigaction( , &sigSetValue, NULL);
	 sigaction( , &sigSetValue, NULL);

	 // Signaler
    signal(SIGINT, SIGhandler);



	 // Print a prompt to indicate input is ready to be accepted
    printf("ndshell>  ");
    // Receive input
	 fgets( buf, max, stdin);
	 // Split the buf and receive the first as the command
	 command = strtok(buf, " "); // split by spaces




	/* EXIT */
	/*
	printf("ndshell: Exiting shell immediately\n");
	

	// START
	printf("ndshell: process %d started\n",pid);
	
	// WAIT
	printf("ndshell: process %d exited normally with status %d\n", pid, );
	printf("ndshell: process %d exited abnormally with signal %d\n", pid, );
	printf("ndshell: No children.\n");

	// WAITFOR
	printf("ndshell: process %d was exited normally with status %d\n", pid, );
	printf("ndshell: No such process.\n");

	// RUN
	printf("ndshell: process %d exited normally with status %d\n", pid, );

	// KILL
	printf("ndshell: process %d started\n", pid);
	printf("ndshell: process %d exited abnormally with signal %d: Killed.\n", pid, );


*/



	 /*
	 // Keep splitting the buf
	 	while (command != NULL){
			command = strtok(NULL, " ");
		}
	 
	  */

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


