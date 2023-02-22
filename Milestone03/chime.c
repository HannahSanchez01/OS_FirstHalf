#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

char g_bKeepLooping = 1;
int i = 0;

#define MAX_THREADS 5

struct ChimeThreadInfo
{
    int        nIndex;
    float      fChimeInterval;
    char       bIsValid;
    pthread_t  ThreadID;
};

struct ChimeThreadInfo  TheThreads[MAX_THREADS];

void * ThreadChime (void * pData)
{
    struct ChimeThreadInfo  * pThreadInfo;

    /* Which chime are we? */
    pThreadInfo = (struct ChimeThreadInfo *) pData;

    while(g_bKeepLooping)
    {
        sleep((unsigned int)(pThreadInfo->fChimeInterval));
        printf("Ding - Chime %d with an interval of %f s!\n", pThreadInfo->nIndex, 
pThreadInfo->fChimeInterval);
    }

    return NULL;
}

#define BUFFER_SIZE 1024

int main (int argc, char *argv[])
{
    char szBuffer[BUFFER_SIZE];
	 char *words[100];
	 char *token;
	 int j;

    /* Set all of the thread information to be invalid (none allocated) */
    for(j=0; j<MAX_THREADS; j++)
    {
        TheThreads[j].bIsValid = 0;
    }

    while(1)
    {
        /* Prompt and flush to stdout */
        printf("CHIME> ");
        fflush(stdout);

        /* Wait for user input via fgets */
        fgets(szBuffer, BUFFER_SIZE, stdin);

		  // Split buf and the first word is the command
		  token = strtok(szBuffer, " \t\n"); // split by spaces, tabs, newline
		  i=0; //reset word counter
		  words[i] = malloc(30 * sizeof(char)); // allocate space for the command string
		  
		  if (token != NULL){ // Prevents seg fault when user only presses enter
		  		strncpy(words[i], token, sizeof(token)); // first word is command
		  }

		  // Keep splitting the buf
	     while ((token != NULL) && (i<=100)){

				i++;
				words[i] = malloc(30 * sizeof(char)); // allocate space for string
				token = strtok(NULL, " \t\n");
				words[i] = token;
		  }


        /* If the command is quit - join any active threads and finish up 
gracefully */
		  if (strncmp(words[0], "quit", sizeof("quit")) == 0){
		  		for (j=0; j<MAX_THREADS; j++){ // join all threads
					if ( TheThreads[j].bIsValid == 1){ // only join if the thread has valid data AKA it was created
						pthread_join( TheThreads[j].ThreadID, NULL);
					}
				}
				
				for (j=0; j<i; j++){ // free malloced memory for words
					free(words[i]);
				}

				return 0;
		  }

        /* If the command is chime, the second argument is the chime number 
(integer) and the 
           third number is the new interval (floating point). If necessary, start 
the thread
           as needed */
		  else if (strncmp(words[0], "chime", sizeof("chime")) == 0){
		  		int index;
				float interval;

				index = atoi(words[1]); // TODO if invalid, this will return 0... something other than atoi might need to be used
				TheThreads[index].nIndex = index; /// TODO ??? what is up with the indexes
				
				if (TheThreads[index].bIsValid == 0){ // if thread is invalid, start a new one!					//TODO
					
					TheThreads[index].bIsValid = 1; // validate the data

					interval = (float) atof(words[2]); // TODO check to make sure interval is a valid float
					TheThreads[index].fChimeInterval = interval;
					pthread_create( &(TheThreads[index].ThreadID), NULL, ThreadChime, TheThreads[index]);
				}

				else{ // just update the existing thread

				}
		  }

		  // Unknown command
		  else{
		  		if (strcmp(words[0],"") != 0){
		  			printf("Unknown command: %s\n",words[0]);
				}
		  }

        /* Optionally, provide appropriate protection against changing the
           chime interval and reading it in a thread at the same time by using a
           mutex.  Note that it is not strictly necessary to do that */


/*
			if (i>0){
				for (j=0; j<i; j++){ // free malloced memory for words
					free(words[i]);
				}
			}
			*/
    }
}
