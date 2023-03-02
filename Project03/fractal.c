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

void compute_image_multithread ( struct FractalSettings * pSettings, struct bitmap * pBitmap, int min_i, int max_i, int min_j, int max_j)
{
	int i,j;

	// For every pixel i,j, in the image...

	for(j=min_j; j<max_j; j++) {
		for(i=min_i; i<max_i; i++) {

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

/* Process all of the arguments as provided as an input and appropriately modify the
   settings for the project 
   @returns 1 if successful, 0 if unsuccessful (bad arguments) */
char processArguments (int argc, char * argv[], struct FractalSettings * pSettings)
{
    static struct option opts[] = {
        {"help", no_argument, 0, 'h'},
        {"xmin", required_argument, 0, 'a'},
        {"xmax", required_argument, 0, 'b'},
        {"ymin", required_argument, 0, 'c'},
        {"ymax", required_argument, 0, 'd'},
        {"maxiter", required_argument, 0, 'm'},
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'h'},
        {"output", required_argument, 0, 'o'},
        {"threads", required_argument, 0, 't'},
        {"row", no_argument, 0, 'r'},
        {"task", no_argument, 0, 's'},
        {0, 0, 0, 0}
    }
    int c, optindex = 0;

    while ((c = getopt_long_only(argc, argv, NULL, opts, &optindex)) != -1){
        switch (c){
            case 'h':
                printf("help message\n");
                return 0;
            case 'a':
                if (atof(optarg) || isNumber(optarg, strlen(optarg))){
                    pSettings->fMinX = atof(optarg);
                }
                else{
                    printf("Error: -xmin requires numeric argument.\n");
                    return 0;
                }
                break;
            case 'b':
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
            case 'c':
                if (atof(optarg) || isNumber(optarg, strlen(optarg))){
                    pSettings->fMinY = atof(optarg);
                }
                else{
                    printf("Error: -ymin requires numeric argument.\n");
                    return 0;
                }
                break;
            case 'd':
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
            case 'm':
                if (atoi(optarg)>0){
                    pSettings->nMaxIter = atoi(optarg);
                }
                else{
                    printf("Error: -maxiter requires positive integer argument.\n");
                    return 0;
                }
                break;
            case 'w':
                if (atoi(optarg)>0){
                    pSettings->nPixelWidth = atoi(optarg);
                }
                else{
                    printf("Error: -width requires positive integer argument.\n");
                    return 0;
                }
                break;
            case 'h':
                if (atoi(optarg)>0){
                    pSettings->nPixelHeight = atoi(optarg);
                }
                else{
                    printf("Error: -height requires positive integer argument.\n");
                    return 0;
                }
                break;
            case 'o':
                if (strlen(optarg)<=MAX_OUTFILE_NAME_LEN){
                    strncpy(pSettings->szOutfile, optarg, MAX_OUTFILE_NAME_LEN);
                }
                else{
                    printf("Error: output filename must be less than %d characters.\n", MAX_OUTFILE_NAME_LEN);
                    return 0;
                }
                break;
            case 't':
                if (atoi(optarg)>0){
                    pSettings->nThreads = atoi(optarg);
                }
                else{
                    printf("Error: -threads requires positive integer argument.\n");
                    return 0;
                }
                break;
            case 'r':
                pSettings->theMode = MODE_THREAD_ROW;
                break;
            case 's':
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

int isNumber(char *str, int len)
{
    for(int i=0; i<len; i++){
        if((str[i]<'-') || (str[i]>'9') || (str[i] == '/')){
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
        if(theSettings.theMode == MODE_THREAD_SINGLE)
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
        else if(theSettings.theMode == MODE_THREAD_ROW)
        {
            /* A row-based approach will not require any concurrency protection */

            int row_size = theSettings.nPixelHeight / theSettings.nThreads; 
            struct bitmap * pBitmap = bitap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);
            pthread_t thread[theSettings.nThreads];

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));

            /* Compute the image */
            for(int i=0; i<theSettings.nThreads){
                compute_image_multithread(&theSettings, pBitmap, 0, theSettings.nPixelWidth, i*row_size, (i+1)*row_size);
            }

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }
        }
        else if(theSettings.theMode == MODE_THREAD_TASK)
        {
            /* For the task-based model, you will want to create some sort of a way that captures the instructions
               or task (perhaps say a startX, startY and stopX, stopY in a struct).  You can have a global array 
               of the particular tasks with each thread attempting to pop off the next task.  Feel free to tinker 
               on what the right size of the work unit is but 20x20 is a good starting point.  You are also welcome
               to modify the settings struct to help you out as well.  
               
               Generally, it will be good to create all of the tasks into that array and then to start your threads
               with them in turn attempting to pull off a task one at a time.  
               
               While we could do condition variables, there is not really an ongoing producer if we create all of
               the tasks at the outset. Hence, it is OK whenever a thread needs something to do to try to access
               that shared data structure with all of the respective tasks.  
               */
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
