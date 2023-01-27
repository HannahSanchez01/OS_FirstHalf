#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int main(int argc, char *argv[])
{
char *filename = malloc(sizeof(char) * 64);
int byteval;
char *temp = malloc(sizeof(char) * 16);






if (argc==1){   //user doesn't give any input
    printf("Enter a filename: ");
    scanf("%s", filename);
    getchar(); //clear ENTER from input buffer
    printf("Enter a byte value to search for: ");
   
    //check if it is in hex
    scanf("%s", temp);
    getchar();
    while((temp[0] != '0') || (temp[1] != 'x')){
        printf("Error: value is not hex. Please enter a hex value, beginning with '0x': ");
        scanf("%s", temp);
        getchar();
    }
    sscanf(temp, "%x", &byteval);
}
else if (argc<4){ //user provides 1 or 2 command-line inputs
    strncpy(filename, argv[1],64);
    if (argc==2){ // user doesn't provide a hex value
        printf("Please enter a hex byte value to search for: ");
        scanf("%s", temp);
        getchar();
    }
    else{ //user gives hex
        strcpy(temp, argv[2]);
    }
    //check if hex
    while((temp[0] != '0') || (temp[1] != 'x')){
        printf("Error: value is not hex. Please enter a hex value, beginning with '0x': ");
        scanf("%s", temp);
        getchar();
    }
    sscanf(temp, "%x", &byteval);
}
else{ // user provides too many command-line inputs
    printf("Too many inputs.\n");
    return -1; // return with an error


}
//get valid filename
FILE *fp = fopen(filename, "r");
while(fp==NULL){
    printf("Error: file not found. Please re-enter the filename, or 'q' to quit: ");
    memset(filename, '\0', 64);
    scanf("%s", filename);
    getchar();
    if (strcmp(filename, "q")){
        return 0;
    }
    fp = fopen(filename, "r");
}


//get file size in bytes
fseek(fp, 0L, SEEK_END);
long int filesize = ftell(fp);
if (filesize > 25*1024){
    printf("Error: The file is over 25kB (file size was %ld).\n", filesize);
    return -1;
}
rewind(fp);


//find the hex values
char *buf = malloc(256);
int bytecount = 0;
while(1){
    if(feof(fp)){
        break;
    }
    fread(buf, 1, 255, fp);
    for (int i=0; i<strlen(buf); i++){ //does this count as byte-by-byte?
        if (buf[i]==byteval){
            bytecount++;
        }
    }
    //clears buf
    memset(buf, '\0', 256);
}


printf("%d\n", bytecount);
free(buf);
free(temp);
free(filename);
fclose(fp);


return 0;
}

