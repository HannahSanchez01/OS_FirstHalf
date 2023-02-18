#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

int pid_list[10] = {0}; //array to store current processes
int pid_index = 0;
int i = 0;

void pid_add(int pid){
	pid_list[pid_index] = pid;
	pid_index++;
}

void pid_remove(int pid){

	int index = -1;

	for(int j=0; j<pid_index; j++){

		if(pid_list[j] == pid){
			pid_list[j] = 0; //remove pid from active list
			index = j;
		}

	}

	if(index != -1){

		for(int j=index; j<pid_index; j++){

			if(j < 9){
				pid_list[j] = pid_list[j+1]; //shift left to keep list condensed
			}

			else{
				pid_list[j] = 0; //if at last index, just make 0
			}
		}

		pid_index--;
	}
}

/* Start */
int start(char **command){

	int cpid;
	int rc = fork();

	if(rc==0){ // child process
		cpid = getpid();
		printf("ndshell: process %d started\n",cpid);
      //printf("Executing: %s\n", command);
      execvp(command[0], command);
		//printf("This shouldn't appear\n");
		pid_remove(cpid);
      	exit(0);
    }

	else if(rc<0){ // fork failed; exit
	 	fprintf(stderr, "fork failed\n");
		exit(1);
	}

    else{ // parent process
		pid_add(rc); // rc is child pid
    }

	return rc; //return child pid so we know about it later
}

/* WAIT */
void wait_func(int *status_ptr){

	pid_t cpid = waitpid(-1, status_ptr, 0);
		
	if (cpid == -1){ // no children
		printf("ndshell: No children.\n");
	}

	else if (WIFEXITED(*status_ptr)){ // if true, normal
		printf("ndshell: process %d exited normally with status %d\n", cpid, WEXITSTATUS(*status_ptr) );
		pid_remove(cpid);
	}
		
	else if (WIFSIGNALED(*status_ptr)){// if true, abnormal
		printf("ndshell: process %d exited abnormally with signal %d\n", cpid, WTERMSIG(*status_ptr));
		pid_remove(cpid);
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
		pid_remove(cpid);
	}

	else if (WIFSIGNALED(*status_ptr)){// if true, abnormal
		printf("ndshell: process %d exited abnormally with signal %d\n", cpid, WTERMSIG(*status_ptr));
		pid_remove(cpid);
	}

	return;
}

// RUN //
void run_func(char **command, int *status_ptr){

	int cpid = start(command);
	wait_for(cpid, status_ptr);
	
	return;
}

// KILL //
void kill_child(int pid, int *status_ptr){

	kill(pid, SIGKILL);
	wait_for(pid, status_ptr);
}

// QUIT //
void quit(int *status_ptr){

	for(int j=pid_index-1; j>=0; j--){ // reverse order for optimization
		kill_child(pid_list[j], status_ptr);
	}

	printf("All child processes complete - exiting the shell.\n");
}

// CONTROL-C
void SIGhandler(int sig){
	int status;
	int pid = pid_list[pid_index-1];
	/* Signal handling function */
	if((pid != 0) && (sig == SIGINT)){
   		printf("Control-C was pressed ... ending most recent process (id: %d)\n", pid);
    	kill_child(pid, &status); // send a kill to last running process?
	}
	else{
		printf("\n"); //formatting
	}
}

// BOUND //
void bound(int time, char **command, int *status_ptr){

	int waittime = 0;
	int pid = start(command);
	int cpid, kill_flag=0;
	do {
		cpid = waitpid(pid, status_ptr, WNOHANG);

		if (cpid == 0) {

			if (waittime < time) {
				sleep(1);
				waittime ++;
			}

			else {
				printf("ndshell: process %d exceeded the time limit, killing it...\n", pid);
				kill_child(pid, status_ptr); 
				kill_flag = 1;
			}
		}

	} while (cpid == 0 && waittime <= time);

	if  (!kill_flag && (WIFEXITED(*status_ptr))){ // if true, normal
		printf("ndshell: process %d was exited normally with status %d\n", cpid, WEXITSTATUS(*status_ptr));
		pid_remove(cpid);
	}

	else if (!kill_flag &&(WIFSIGNALED(*status_ptr))){// if true, abnormal
		printf("ndshell: process %d exited abnormally with signal %d\n", cpid, WTERMSIG(*status_ptr));
		pid_remove(cpid);
	}
}

int main(void)
{
	struct sigaction sigSetValue;
	sigSetValue.sa_handler = SIGhandler;

	int max = 1000;
	char *token;
	char *words[100];
	char *command[100];
    char buf[max];
	int pid;
	int time;
	int status;
   
	// Set up sigactions   ///control C and Kill.
	// Kill
	//sigaction(SIGKILL, &sigSetValue, NULL); // do we need this?

	// Signaler C 
	sigaction(SIGINT, &sigSetValue, NULL);

	int done = 0;
	while( !done) {
		if (buf[0]){
			memset(buf, '\0', (size_t) max);
		} 

		// Print a prompt to indicate input is ready to be accepted
		printf("ndshell>  ");

		// Receive input
		if (fgets( buf, max, stdin) == NULL){
			continue;
		}

		// Split the buf and receive the first as the command
		token = strtok(buf, " \t\n"); // split by spaces
		i=0; //reset word counter
		words[i] = malloc(30 * sizeof(char)); // allocate space for the command string
		strcpy(words[i], token); // first word is command

		// Keep splitting the buf
		while ((token != NULL) && (i<=100)){

			i++;
			words[i] = malloc(30 * sizeof(char)); // allocate space for string
			token = strtok(NULL, " \t\n");
			words[i] = token;
		}

		/* EXIT */
		if (strncmp(words[0], "exit", sizeof("exit")) == 0){

			printf("ndshell: Exiting shell immediately\n");
			return 0;
		}

		// START //
		else if (strncmp(words[0], "start", sizeof("start")) == 0){

			for (int j=1; j<i; j++){
				command[j-1] = words[j];
				command[j] = NULL;
			}

			start(command);
			usleep(10000);
		}
		
		// WAIT 
		else if (strncmp(words[0], "wait", sizeof("wait")) == 0){

			wait_func(&status);
		}
		
		// WAITFOR //blocks, wait for specifc waitpid
		else if (strncmp(words[0], "waitfor", sizeof("waitfor")) == 0){

			pid = atoi(words[1]); // make the input an int from string
			wait_for(pid, &status);
		}
		
		// RUN	
		else if (strncmp(words[0], "run", sizeof("run")) == 0){

			for (int j=1; j<i; j++){
				command[j-1] = words[j];
				command[j] = NULL;
			}

			run_func(command, &status);
		}

		// BOUND
		else if (strncmp(words[0], "bound", sizeof("run")) == 0){

			time = atoi(words[1]);

			for (int j=2; j<i; j++){
				command[j-2] = words[j];
				command[j-1] = NULL;
			}

			bound(time, command, &status);
		}

		// KILL
		else if (strncmp(words[0], "kill", sizeof("kill")) == 0){

			pid = atoi(words[1]);
			kill_child(pid, &status);
		}

		// QUIT
		else if (strncmp(words[0], "quit", sizeof("quit")) == 0){

			quit(&status);
			return 0;
		}

		else{ // error check for undefined command

			printf("ndshell: unknown command: %s\n",words[0]);
		}
	}

    return 0;
}
