#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

void SIGhandler(int sig)
{
    printf("Control-C was pressed. Exiting program.\n");
    exit(0);
}

int main(void)
{
    char command[64];
    char c;
    
    signal(SIGINT, SIGhandler);

    printf("Execute? ");
    scanf("%64[^\n]", command); //get input; limit length to avoid overflow
    while (((c=getchar()) != '\n') && (c != EOF)); //clear input buffer

    if(fork()==0){ //child process
        printf("Executing: %s\n", command);
        system(command);
        exit(0);
    }
    else{
        wait(NULL);
        printf("Execution complete!\n");
    }

    return 0;
}