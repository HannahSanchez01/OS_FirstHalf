#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

int i = 0;
void SIGhandler(int sig)
{
	 /* Signal handling function */
	 if(sig == SIGINT){
   	 printf("Control-C was pressed ... exiting\n");
    	exit(0); // send a kill to last running process?
	}

	else if (sig == SIGKILL){
		/// get the pid in the signal handler TODO
	}
}

/* Start */
void start(int* status_ptr)
{
	int cpid;
	int rc = fork();

   if(rc==0){ // child process
		cpid = getpid();
		printf("ndshell: process %d started\n",cpid);
      //printf("Executing: %s\n", command);
      //system(command);
      exit(0);
    }

	 else if(rc<0){ // fork failed; exit
	 	fprintf(stderr, "fork failed\n");
		exit(1);
	 }

    else{ // parent process
		// check                     TODO
       waitpid(-1, status_ptr, WNOHANG);   /// NONBLOCKING HERE???!!!!!
        printf("Execution complete!\n");
    }

	 return;
}

/* WAIT */
void wait_func(int * status_ptr){
	pid_t cpid = waitpid(-1, status_ptr, 0);
		
	if (cpid == -1){ // no children
		printf("ndshell: No children.\n");
	}

	else if  (WIFEXITED(*status_ptr)){ // if true, normal
		printf("ndshell: process %d exited normally with status %d\n", cpid, WEXITSTATUS(*status_ptr) );
	}
		
	else if (WIFSIGNALED(*status_ptr)){// if true, abnormal
		printf("ndshell: process %d exited abnormally with signal %d\n", cpid, WTERMSIG(*status_ptr));
	}

	return;
}

/* WAIT FOR */
void wait_for(int pid, int *status_ptr){

	pid_t cpid = waitpid(pid, status_ptr, 0); 
		
	if (cpid == -1){ // process does not exist
		printf("ndshell: No such process.\n");
	}

	else if  (WIFEXITED(*status_ptr)){ // if true, normal
		printf("ndshell: process %d was exited normally with status %d\n", cpid, WEXITSTATUS(*status_ptr));
	}

	return;
}

/*
// RUN //
void run_func(int *status_ptr, int pid){
	start(status_ptr);
	wait_for(pid, status_ptr);
	

	return;
}
*/


int main(void)
{

	 struct sigaction sigSetValue;
	 sigSetValue.sa_handler = SIGhandler;

	 int max = 64;
	 char *token;
	 char *words[100];
    char buf[max];
    int c, rc;
	 int cpid, pid;
	 int *status_ptr;
   
	 // Set up sigactions   ///control C and Kill.
	 // Kill
	 sigaction(SIGKILL, &sigSetValue, NULL);

	// does this need to be sigaction instead? TODO
	 // Signaler C 
    //signal(SIGINT, SIGhandler);
	 sigaction(SIGINT, &sigSetValue, NULL);

	 int done = 0;
	 while( !done) {


	// TODO Trying to clear the buffer?
	 if (buf){
		printf("%s\n",buf);
	 	memset(buf, '\0', max);
	 }

	
	 

	 // Print a prompt to indicate input is ready to be accepted
    printf("ndshell>  ");
    // Receive input
	 if (fgets( buf, max, stdin) == NULL){
	 	printf("Did you get here?\n");
		//Stop if fgets returns NULL
	 }
	 // Split the buf and receive the first as the command
	 token = strtok(buf, " \t\n"); // split by spaces
	 words[i] = malloc(30 * sizeof(char)); // allocate space for the command string
	 strcpy(words[i], token); // first word is command
	 // Keep splitting the buf
	 while (token != NULL){
		 i++;
	    words[i] = malloc(30 * sizeof(char)); // allocate space for string
	    words[i] = strtok(NULL, " ");
	 }
	 
	// TODO:
	//  /* OPTION WORKING*/
	//  //! Option in progres//!

	/* EXIT */
	if (strncmp(words[0], "exit", sizeof("exit")) == 0){
		printf("ndshell: Exiting shell immediately\n");
		return 0;
	}

	// check
	//// DOESN'T LET THE SHELL KEEP RUNNING: make it nonblocking
	//! START //!
	else if (strncmp(words[0], "start", sizeof("start")) == 0){
		start(status_ptr);
	}
	
	// WAIT 
	else if (strncmp(words[0], "wait", sizeof("wait")) == 0){
		wait_func(status_ptr);
	}
	
	// WAITFOR //blocks, wait for specifc waitpid
	else if (strncmp(words[0], "waitfor", sizeof("waitfor")) == 0){
		pid = atoi(words[1]); // make the input an int from string
		wait_for(pid, status_ptr);
	}


	
	// RUN	
	else if (strncmp(words[0], "run", sizeof("run")) == 0){
		printf("%d\n",i);
		//run_func(pid, status_ptr);
		// make arg list
/*
 		
		if (fork() == 0){ // child
			int status_code = execvp(words[1], arg_list);
			if (status_code == -1){
				printf("Terminated incorrectly\n");
				return -1;
			}
		}
		else{ // parent
			
		}
*/
	}


	else if (strncmp(words[0], "logout", sizeof("logout")) == 0){
		done = 1; 
		break;
	}

	else{ // error check for undefined command
		printf("Undefined command.\n");
	}

	//printf("ndshell: process %d exited normally with status %d\n", pid, );
	// combine start and waitfor: fork and use execvp()
	//
	/* execvp notes:
	 * use execvp on a new process after forking
	 *	the arg list must be NULL terminated!
	 * anything that comes after will NOT execute
	 * returns -1 if failed
	 */


	/*
	// KILL
	printf("ndshell: process %d started\n", pid);
	printf("ndshell: process %d exited abnormally with signal %d: Killed.\n", pid, );


*/



	// QUIT
	// to kill all
	/*
	 kill(-1, SIGKILL);

	 */



		// Don't think we need this anymore
	 // clear input buffer
    //while (((c=getchar()) != '\n') && (c != EOF)){}
	}

    return 0;
}


