#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "globals.h"
#include "utils.h"
#include "assemble.h"
#include "label.h"

#define FILENAMEMAXLENGTH 128

int main(int argc, char *argv[])
{
    char outfilename[FILENAMEMAXLENGTH];

    if(argc < 2){
        printf("Usage: asm <file.s> [-d]\n");
        exit(1);
    }
    infile = fopen(argv[1],"r");
    if(infile == NULL){
        printf("Error opening \"%s\"\n",argv[1]);   
        exit(1);             
    }
    
    // prepare output filename
    strcpy(outfilename, argv[1]);
    remove_ext(outfilename, '.', '/');
    strcat(outfilename, ".bin");
    
    outfile = fopen(outfilename, "wb");
    if(outfile == NULL){
        printf("Error opening \"%s\"\n",outfilename);
        fclose(infile);
        exit(1);
    }

    debug_enabled = false;
    listing_enabled = false;
    if((argc == 3) && (strcmp(argv[2], "-d") == 0)) debug_enabled = true;
    if((argc == 3) && (strcmp(argv[2], "-l") == 0)) listing_enabled = true;

    // Init tables
    init_instruction_table();
    init_label_table();
    outputbufferptr = outputbuffer;

    // Assemble input to output
    assemble(infile, outfile);

    if(global_errors) printf("Error in input\n");
    
    // Cleanup
    fclose(infile);
    fclose(outfile);
    return 0;
}