#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){

	char *input_filename = malloc(sizeof(char) * 64);
	char *output_filename = malloc(sizeof(char) * 64);
	char *file_extension = malloc(sizeof(char) * 5);
	// Default file extension = .bf
	strncpy(file_extension, ".bf", 5);

	int max_size = 25*1024;
	int e_bfr = 0, e_r = 0, e_o = 0; // enable var 0 or 1 for flags bfr, r, and o

	/* Handle Flags */
	int c, optindex=0;
	static struct option longopts[] = {
    		{"help", no_argument, 0, 'h'},
    		{"maxsize", required_argument, 0, 'm'},
    		{"bfr", no_argument, 0, 'b'},
    		{0, 0, 0, 0}
	};

	while ((c = getopt_long_only(argc, argv, "o:r", longopts, &optindex)) != -1){
    	    switch (c){
        	case 'h':
            		printf("Usage: ./bitflip <filename> [-help] [-o XXX] [-maxsize XXX] [-bfr|-r]\n");
            		printf("filename: the name of the input file\n");
            		printf("-help: brings up this message\n");
            		printf("-o XXX: uses the specified file name (XXX) instead of the default (note: does not work in directory mode)\n");
            		printf("-maxsize XXX: specifies a new maximum file size for the input (default=25kB)\n");
            		printf("-bfr: reverse the order of the bytes and bit-flip every bit. May not be used with -r\n");
            		printf("-r: reverse the order of the bytes without bit-flipping. May not be used with -bfr\n");
				
			free(input_filename);
			free(output_filename);
			free(file_extension);
		
            		return 0;
        	case 'm':
            		// Check for size as an int
            		if (atoi(optarg)){
                		max_size = atoi(optarg);
            		}
            		else{
                		printf("Error: Valid number not specified for -maxsize\n");
				free(input_filename);
				free(output_filename);
				free(file_extension);
				return -1;
            		}
            		break;
        	case 'b':
            		// bfr only works when r is not enabled
            		if (e_r != 1){
                	e_bfr = 1;
                	// Change the file extension
                	strncpy(file_extension, ".bfr", 5);
            		}
            		else{
                		printf("Error: cannot use -r and -bfr at the same time!\n");
				free(input_filename);
				free(output_filename);
				free(file_extension);
                	return -1;
            		}
            		break;
        	case 'o':
            		// Change output file name
            		strncpy(output_filename, optarg, 64);
            		e_o = 1;
            		break;
        	case 'r':
            		// r only works when bfr is not enabled
            		if (e_bfr != 1){
                	e_r = 1; 
                	// Change the file extension
                	strncpy(file_extension, ".r", 5);
            		}
            		else{
                		printf("Error: cannot use -r and -bfr at the same time!\n");
				free(input_filename);
				free(output_filename);
				free(file_extension);
                		return -1;
            		}
            		break;
        	case '?':
            		if ((optopt == 'o') || (optopt == 'm')){
                		printf("Error: Option -%c requires an argument.\n", optopt);
				free(input_filename);
				free(output_filename);
				free(file_extension);
				return -1;
            		}
            		else{
                		printf("Error: unknown option -%c.\n", optopt);
				free(input_filename);
				free(output_filename);
				free(file_extension);
				return -1;
            		}
        	default:
            		abort();
    	    }
	}

	strncpy(input_filename, argv[optind], 64);

	struct stat filestat;
	stat(input_filename, &filestat);
	DIR *dir = NULL;
	FILE *i_fp = NULL;
	struct dirent *file;


	// Check for file versus directory
	if (S_ISDIR (filestat.st_mode)){
    	    dir = opendir(input_filename);
    	    // Can't use -o with directory
		if(e_o == 1){
			printf("Error: -o cannot be used in directory mode\n");
			free(input_filename);
			free(output_filename);
			free(file_extension);
			return -1;
		}
	}
	else{
    	/* Check input file access */
    	    i_fp = fopen(input_filename, "r");
    	    if (i_fp==NULL){
       		printf("Error: The file %s does not exist or is not accessible.\n",input_filename);
		free(input_filename);
		free(output_filename);
		free(file_extension);
        	return -1;
    	    }
	}

	//// Single File Mode
	if(dir==NULL){ 
		/* Check output file access and open for writing */
		// Without -o flag, append .bf to the inputFile for outputFile name
		if (!e_o){
    			strncpy(output_filename, input_filename, 65);
    			strcat(output_filename, file_extension);
		}
		// Check if .bf file already exists for output!
		FILE *o_fp = fopen(output_filename, "r");
		if(o_fp!=NULL){
    			printf("Error: %s already exists!\n",output_filename);
    			fclose(o_fp);
			fclose(i_fp);
			free(input_filename);
			free(output_filename);
			free(file_extension);
    			return -1;
		}
		else{
   	 		o_fp = fopen(output_filename, "w+");
		}

		/* Check file size for number of bytes */
		fseek(i_fp, 0L, SEEK_END);
		long int filesize = ftell(i_fp);
		if (filesize > (long int)max_size){
    		printf("Error: Input file %s was too large (max size is %d, file size was %ld).\n", input_filename, max_size, filesize);
			fclose(i_fp);
			fclose(o_fp);
			free(input_filename);
			free(output_filename);
			free(file_extension);
    			return -1;
		}
		else{
			printf("Input: %s was %ld bytes\n",input_filename, filesize);
		}
		rewind(i_fp);

		/* Find the hex values */
		char *buf = calloc((long unsigned int)filesize + 1, sizeof(char));
		//calloc to initalize as ‘\0’, filesize+1 to make room for terminating character
		int byteVal;
		// Read file into buf
		fread(buf, 1, (long unsigned int)filesize, i_fp);
    		// buf[i] is each byte
		if (e_r){  // reversal
    			for (int i=(int)strlen(buf)-1; i>=0; i--){
        			fprintf(o_fp, "%c", buf[i]);
    			}
		}
		else if (e_bfr){  // bit-flip reversal
    			for (int i=(int)strlen(buf)-1; i>=0; i--){
        			byteVal = buf[i];
       		 		byteVal = byteVal ^ 0xFF;
        			fprintf(o_fp, "%c", byteVal);
    			}
		}
		else{ // bit-flip
    			for (int i=0; i<strlen(buf); i++){ 
        			byteVal = buf[i];
        			byteVal = byteVal ^ 0xFF;
        			fprintf(o_fp, "%c", byteVal);
       				//printf("Comparing the toggled values: %02X vs. %02X\n", byteVal, byteVal ^ 0xFF);
    			}
		}
		
		// Success Message
		long int outfilesize = ftell(o_fp);
		if (outfilesize == filesize){ // check that input and output have same byte size
			if (e_bfr){
				printf("Output: %s was output successfully (bitflip, reverse)\n",output_filename);
			}
			else if (e_r){
				printf("Output: %s was output successfully (reverse)\n",output_filename);
			}
			else{
				printf("Output: %s was output successfully\n",output_filename);
			}
		}
		else{
			printf("%s was not output successfully: bytes do not match %s\n",output_filename, input_filename);
		}

		free(buf);
		fclose(i_fp);
		fclose(o_fp);
	}
	
	//// Directory Mode
	else if (dir != NULL){
		char *dirname = malloc(64);
		strncpy(dirname, input_filename, 64); //copy directory name
		strcat(dirname, "/"); //add slash so it reads <directory>/<file>
	    while((file=readdir(dir))){
    		strncpy(input_filename, dirname, 64);
    		strcat(input_filename, file->d_name);
			if ((stat(input_filename, &filestat)==0) && (S_ISREG(filestat.st_mode))){
    				i_fp = fopen(input_filename, "r"); //only open if actually a file
			}
			else{
    				continue; //continue skips to next file instead of ending program
			}

			// Create the output file name with proper extension
			strncpy(output_filename, input_filename, 64);
			strcat(output_filename, file_extension);

			// Check if .bf file already exists for output!
			FILE *o_fp = fopen(output_filename, "r");
			if(o_fp!=NULL){
    				printf("Error: %s already exists!\n",output_filename);
    				fclose(o_fp);
    				fclose(i_fp);
    				continue;
			}
			else{
    				o_fp = fopen(output_filename, "w+");
			}

			/* Check file size for number of bytes */
			fseek(i_fp, 0L, SEEK_END);
			long int filesize = ftell(i_fp);
			if (filesize > (long int)max_size){
    				printf("Error: Input file %s was too large (max size is %d, file size was %ld).\n", input_filename, max_size, filesize);
    				fclose(o_fp);
    				fclose(i_fp);
    				continue;
			}
			else {
				printf("Input: %s was %ld bytes\n",input_filename, filesize);
			}
			rewind(i_fp);




			/* Find the hex values */
			char *buf = calloc((long unsigned int)filesize + 1, sizeof(char));
			int byteVal;
			fread(buf, 1, (long unsigned int)filesize, i_fp);
    			// buf[i] is each byte
			if (e_r){
    				for (int i=(int)strlen(buf)-1; i>=0; i--){
        				fprintf(o_fp, "%c", buf[i]);
    				}
			}			
			else if (e_bfr){
   				for (int i=(int)strlen(buf)-1; i>=0; i--){
        				byteVal = buf[i];
       				 	byteVal = byteVal ^ 0xFF;
       				 	fprintf(o_fp, "%c", byteVal);
    				}
			}
			else{
    				for (int i=0; i<strlen(buf); i++){ //does this count as byte-by-byte?
        				byteVal = buf[i];
        				byteVal = byteVal ^ 0xFF;
        				fprintf(o_fp, "%c", byteVal);
        				//printf("Comparing the toggled values: %02X vs. %02X\n", byteVal, byteVal ^ 0xFF);
    				}
			}

			// Success Message
			long int outfilesize = ftell(o_fp);
			if (outfilesize == filesize){ // check that input and output have same byte size
				if (e_bfr){
					printf("Output: %s was output successfully (bitflip, reverse)\n",output_filename);
				}
				else if (e_r){
					printf("Output: %s was output successfully (reverse)\n",output_filename);
				}
				else{
					printf("Output: %s was output successfully\n",output_filename);
				}
			}
			else{
				printf("%s was not output successfully: bytes do not match %s\n",output_filename, input_filename);
			}
			free(buf);
			fclose(i_fp);
			fclose(o_fp);
		}
		free(dirname);
		closedir(dir);
	}

	free(input_filename);
	free(output_filename);
	free(file_extension);
	return 0;
}


