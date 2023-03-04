/*
fractal.c - Mandelbrot fractal generation
Starting code for CSE 30341 Project 3 - Spring 2023
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <complex.h>
#include <pthread.h>
#include <getopt.h>

#include "bitmap.h"
#include "fractal.h"



 //or task (perhaps say a startX, startY and stopX, stopY in a struct).  You can have a global array 

// Task Struct
struct p_tasks{
	int startX;
	int startY;
	int stopX;
	int stopY;
};

// Global Task List
struct p_tasks tasks[9000];
int num_tasks = 0;

// Mutex lock
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*
Compute the number of iterations at point x, y
in the complex space, up to a maximum of maxiter.
Return the number of iterations at that point.

This example computes the Mandelbrot fractal:
z = z^2 + alpha

Where z is initially zero, and alpha is the location x + iy
in the complex plane.  Note that we are using the "complex"
numeric type in C, which has the special functions cabs()
and cpow() to compute the absolute values and powers of
complex values.
*/

static int compute_point( double x, double y, int max )
{
	double complex z = 0;
	double complex alpha = x + I*y;

	int iter = 0;

	while( cabs(z)<4 && iter < max ) {
		z = cpow(z,2) + alpha;
		iter++;
	}

	return iter;
}

/*
Compute an entire image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax).

HINT: Generally, you will want to leave this code alone and write your threaded code separately

*/

void compute_image_singlethread ( struct FractalSettings * pSettings, struct bitmap * pBitmap)
{
	int i,j;

	// For every pixel i,j, in the image...

	for(j=0; j<pSettings->nPixelHeight; j++) {
		for(i=0; i<pSettings->nPixelWidth; i++) {

			// Scale from pixels i,j to coordinates x,y
			double x = pSettings->fMinX + i*(pSettings->fMaxX - pSettings->fMinX) / pSettings->nPixelWidth;
			double y = pSettings->fMinY + j*(pSettings->fMaxY - pSettings->fMinY) / pSettings->nPixelHeight;

			// Compute the iterations at x,y
			int iter = compute_point(x,y,pSettings->nMaxIter);

			// Convert a iteration number to an RGB color.
			// (Change this bit to get more interesting colors.)
			int gray = 255 * iter / pSettings->nMaxIter;

         // Set the particular pixel to the specific value
			// Set the pixel in the bitmap.
			bitmap_set(pBitmap,i,j,gray);
		}
	}
}

void * compute_image_multithread (void *args)
{
    struct ThreadArgs *pargs = (struct ThreadArgs *) args;
	int i,j;

	// For every pixel i,j, in the image...

	for(j=pargs->min_j; j<pargs->max_j; j++) { // manually set all 4 bounds for task mode
		for(i=pargs->min_i; i<pargs->max_i; i++) {
            // actual functionality is the same as single thread, just w/ adjusted variables bc threads
			// Scale from pixels i,j to coordinates x,y
			double x = pargs->pSettings->fMinX + i*(pargs->pSettings->fMaxX - pargs->pSettings->fMinX) / pargs->pSettings->nPixelWidth;
			double y = pargs->pSettings->fMinY + j*(pargs->pSettings->fMaxY - pargs->pSettings->fMinY) / pargs->pSettings->nPixelHeight;

			// Compute the iterations at x,y
			int iter = compute_point(x,y,pargs->pSettings->nMaxIter);

			// Convert a iteration number to an RGB color.
			// (Change this bit to get more interesting colors.)
			int gray = 255 * iter / pargs->pSettings->nMaxIter;

         // Set the particular pixel to the specific value
			// Set the pixel in the bitmap.
			bitmap_set(pargs->pBitmap,i,j,gray);
		}
	}
    return 0;
}

void * compute_image_tasks (void *args)
{
    struct ThreadArgs *pargs = (struct ThreadArgs *) args;
	while (num_tasks>0) // keep pulling tasks
	{ 
		pthread_mutex_lock(&lock); // lock and must unlock below!
        pargs->min_i = tasks[num_tasks-1].startX;
        pargs->max_i = tasks[num_tasks-1].stopX;
        pargs->min_j = tasks[num_tasks-1].startY;
        pargs->max_j = tasks[num_tasks-1].stopY;

		num_tasks--; // remove from the total count

		pthread_mutex_unlock(&lock); // unlock the lock from above!

		compute_image_multithread((void *) pargs); 

	}
	return 0;
}

int isNumber(char *str, int len) // make arg processing easier
{
    for(int i=0; i<len; i++){
        if((str[i]<'-') || (str[i]>'9') || (str[i] == '/')){
            return 0;
        }
    }
    return 1;
}

/* Process all of the arguments as provided as an input and appropriately modify the
   settings for the project 
   @returns 1 if successful, 0 if unsuccessful (bad arguments) */
char processArguments (int argc, char * argv[], struct FractalSettings * pSettings)
{
    static struct option opts[] = { // needed for getopt
        {"help", no_argument, 0, 'h'},
        {"xmin", required_argument, 0, 'a'},
        {"xmax", required_argument, 0, 'b'},
        {"ymin", required_argument, 0, 'c'},
        {"ymax", required_argument, 0, 'd'},
        {"maxiter", required_argument, 0, 'm'},
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'y'},
        {"output", required_argument, 0, 'o'},
        {"threads", required_argument, 0, 't'},
        {"row", no_argument, 0, 'r'},
        {"task", no_argument, 0, 's'},
        {0, 0, 0, 0}
    };
    int c, optindex = 0;

    while ((c = getopt_long_only(argc, argv, "", opts, &optindex)) != -1){ // argparsing with getopt
        switch (c){
            case 'h':
                printf("help message\n");
                return 0; // return 0 so that main() doesn't continue
            case 'a': // xmin
                if (atof(optarg) || isNumber(optarg, strlen(optarg))){
                    pSettings->fMinX = atof(optarg);
                }
                else{
                    printf("Error: -xmin requires numeric argument.\n");
                    return 0;
                }
                break;
            case 'b': // xmax
                if (atof(optarg) || isNumber(optarg, strlen(optarg))){
                    if (atof(optarg) > pSettings->fMinX){
                        pSettings->fMaxX = atof(optarg);
                    }
                    else{
                        printf("Error: -xmax argument must be greater than -xmin argument.\n");
                        return 0;
                    }
                }
                else{
                    printf("Error: -xmax requires numeric argument.\n");
                    return 0;
                }
                break;
            case 'c': // ymin
                if (atof(optarg) || isNumber(optarg, strlen(optarg))){
                    pSettings->fMinY = atof(optarg);
                }
                else{
                    printf("Error: -ymin requires numeric argument.\n");
                    return 0;
                }
                break;
            case 'd': // ymax
                if (atof(optarg) || isNumber(optarg, strlen(optarg))){
                    if (atof(optarg) > pSettings->fMinY){
                        pSettings->fMaxY = atof(optarg);
                    }
                    else{
                        printf("Error: -ymax argument must be greater than -ymin argument.\n");
                        return 0;
                    }
                }
                else{
                    printf("Error: -ymax requires numeric argument.\n");
                    return 0;
                }
                break;
            case 'm': // max iter
                if (atoi(optarg)>0){
                    pSettings->nMaxIter = atoi(optarg);
                }
                else{
                    printf("Error: -maxiter requires positive integer argument.\n");
                    return 0;
                }
                break;
            case 'w': // width
                if (atoi(optarg)>0){
                    pSettings->nPixelWidth = atoi(optarg);
                }
                else{
                    printf("Error: -width requires positive integer argument.\n");
                    return 0;
                }
                break;
            case 'y': // height
                if (atoi(optarg)>0){
                    pSettings->nPixelHeight = atoi(optarg);
                }
                else{
                    printf("Error: -height requires positive integer argument.\n");
                    return 0;
                }
                break;
            case 'o': // output
                if (strlen(optarg)<=MAX_OUTFILE_NAME_LEN){
                    strncpy(pSettings->szOutfile, optarg, MAX_OUTFILE_NAME_LEN);
                }
                else{
                    printf("Error: output filename must be less than %d characters.\n", MAX_OUTFILE_NAME_LEN);
                    return 0;
                }
                break;
            case 't': // threads
                if (atoi(optarg)>0){
                    pSettings->nThreads = atoi(optarg);
                }
                else{
                    printf("Error: -threads requires positive integer argument.\n");
                    return 0;
                }
                break;
            case 'r': // row 
                pSettings->theMode = MODE_THREAD_ROW;
                break;
            case 's': // task
                pSettings->theMode = MODE_THREAD_TASK;
                break;
            case '?':
                printf("Invalid option specified.\n");
                return 0;
            default:
                return 0;
        }
    }
    return 1;
}

int main( int argc, char *argv[] )
{
    struct FractalSettings  theSettings;

	// The initial boundaries of the fractal image in x,y space.
    theSettings.fMinX = DEFAULT_MIN_X;
    theSettings.fMaxX = DEFAULT_MAX_X;
    theSettings.fMinY = DEFAULT_MIN_Y;
    theSettings.fMaxY = DEFAULT_MAX_Y;
    theSettings.nMaxIter = DEFAULT_MAX_ITER;

    theSettings.nPixelWidth = DEFAULT_PIXEL_WIDTH;
    theSettings.nPixelHeight = DEFAULT_PIXEL_HEIGHT;

    theSettings.nThreads = DEFAULT_THREADS;
    theSettings.theMode  = MODE_THREAD_SINGLE;
    
    strncpy(theSettings.szOutfile, DEFAULT_OUTPUT_FILE, MAX_OUTFILE_NAME_LEN);

    /* TODO: Adapt your code to use arguments where the arguments can be used to override 
             the default values 

        -help         Display the help information
        -xmin X       New value for x min
        -xmax X       New value for x max
        -ymin Y       New value for y min
        -ymax Y       New value for y max
        -maxiter N    New value for the maximum number of iterations (must be an integer)     
        -width W      New width for the output image
        -height H     New height for the output image
        -output F     New name for the output file
        -threads N    Number of threads to use for processing (default is 1) 
        -row          Run using a row-based approach        
        -task         Run using a thread-based approach

        Support for setting the number of threads is optional

        You may also appropriately apply reasonable minimum / maximum values (e.g. minimum image width, etc.)
    */
    

   /* Are there any locks to set up? */


   if(processArguments(argc, argv, &theSettings))
   {
        /* Dispatch here based on what mode we might be in */
        if(theSettings.theMode == MODE_THREAD_SINGLE)  // Single thread
        {
            /* Create a bitmap of the appropriate size */
            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));

            /* Compute the image */
            compute_image_singlethread(&theSettings, pBitmap);

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }            
        }
        else if(theSettings.theMode == MODE_THREAD_ROW) // Multi-thread by row
        {
            /* A row-based approach will not require any concurrency protection */

            int row_size = theSettings.nPixelHeight / theSettings.nThreads; // figure out how big each band is
            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);
            struct ThreadArgs targs[theSettings.nThreads]; 

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));


            /* Compute the image */
            for(int i=0; i<theSettings.nThreads; i++){
                targs[i].pSettings = &theSettings; // setting up struct for pthread
                targs[i].pBitmap = pBitmap;
                targs[i].min_i = 0;
                targs[i].max_i = theSettings.nPixelWidth;
                targs[i].min_j = i*row_size;
                targs[i].max_j = (i+1)*row_size;
                if(i == theSettings.nThreads-1){
                    targs[i].max_j = theSettings.nPixelHeight; // fix rounding error for non-divisors
                }
                pthread_create(&targs[i].threadID, NULL, compute_image_multithread, (void *) &targs[i]);
            }
            for(int i=0; i<theSettings.nThreads; i++){
                pthread_join(targs[i].threadID, NULL);
            }

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }
        }
        else if(theSettings.theMode == MODE_THREAD_TASK) // Multi-thread by tasks
        {
			// Decide task size
            int task_row_size = 20;
			int task_col_size = 20;

            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);
            struct ThreadArgs targs[theSettings.nThreads]; 

            // Fill task global array
            for(int i=0; i<theSettings.nPixelWidth/task_col_size; i++){
                for(int j=0; j<theSettings.nPixelHeight/task_row_size; j++){ 
                    // col
                    tasks[num_tasks].startX = i*task_col_size;
                    tasks[num_tasks].stopX = (i+1)*task_col_size;

                    if (i == ((theSettings.nPixelWidth/task_col_size)-1)){ // final task in each row
                        tasks[num_tasks].stopX = theSettings.nPixelWidth; // force to boundary
                    }
                    
                    // row
                    tasks[num_tasks].startY = j*task_row_size;
                    tasks[num_tasks].stopY = (j+1)*task_row_size;

                    if (j == ((theSettings.nPixelHeight/task_row_size)-1)){ // final task in each col
                        tasks[num_tasks].stopY = theSettings.nPixelHeight; // force to boundary
                    }

                    // increment num of tasks
                    num_tasks++;
                }
            }

            /* Fill the bitmap with dark green */
            bitmap_reset(pBitmap,MAKE_RGBA(0,255,0,0));

            /* Compute the image */
            for(int i=0; i<theSettings.nThreads; i++){

                targs[i].pSettings = &theSettings; // setting up struct for pthread
                targs[i].pBitmap = pBitmap;

				// This differs from rows. We are using rectangles now
                targs[i].min_i = tasks[i].startX;
                targs[i].max_i = tasks[i].stopX;
                targs[i].min_j = tasks[i].startY;
                targs[i].max_j = tasks[i].stopY;

                pthread_create(&targs[i].threadID, NULL, compute_image_tasks, (void *) &targs[i]);
            }

            for(int i=0; i<theSettings.nThreads; i++){
                pthread_join(targs[i].threadID, NULL);
            }

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }
        }
        else 
        {
            /* Uh oh - how did we get here? */
        }
   }
   else
   {
        /* Probably a great place to dump the help */

        /* Probably a good place to bail out */
        exit(-1);
   }

    /* TODO: Do any cleanup as required */

	return 0;
}
