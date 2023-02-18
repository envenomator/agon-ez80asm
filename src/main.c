#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "globals.h"
#include "utils.h"
#include "assemble.h"
#include "label.h"

#include <time.h>
#include <sys/time.h>

#define FILENAMEMAXLENGTH 128

bool openfiles(char *basename) {
    char outfilename[FILENAMEMAXLENGTH];
    char localsfilename[FILENAMEMAXLENGTH];
    char anonymousfilename[FILENAMEMAXLENGTH];

    infile = fopen(basename,"r");
    if(infile == NULL){
        printf("Error opening \"%s\"\n",basename);   
        return false;
    }
    
    // prepare output filename
    strcpy(outfilename, basename);
    remove_ext(outfilename, '.', '/');
    strcpy(localsfilename, outfilename);
    strcpy(anonymousfilename,outfilename);
    strcat(outfilename, ".bin");
    strcat(localsfilename, ".lbls");
    strcat(anonymousfilename, ".anolbls");
    
    outfile = fopen(outfilename, "wb");
    if(outfile == NULL){
        printf("Error opening \"%s\"\n",outfilename);
        fclose(infile);
        return false;
    }
    locals = fopen(localsfilename, "wb+");
    if(locals == NULL) {
        printf("Error opening \"%s\"\n",localsfilename);
        fclose(infile);
        fclose(outfile);
        return false;
    }
    anonlabels = fopen(anonymousfilename, "wb+");
    if(anonlabels == NULL) {
        printf("Error opening \"%s\"\n", anonymousfilename);
        fclose(infile);
        fclose(outfile);
        fclose(locals);
        return false;
    }
    return true;
}

void closefiles() {
   // Cleanup
    fclose(infile);
    fclose(outfile);
    fclose(locals);
    fclose(anonlabels);
    //remove(localsfilename);
    //remove(anonymousfilename);
}

int main(int argc, char *argv[])
{
    struct timeval stop, start;

    if(argc < 2){
        printf("Usage: asm <file.s> [-d]\n");
        exit(1);
    }

    if(!openfiles(argv[1])) return 0;

    debug_enabled = false;
    listing_enabled = false;
    if((argc == 3) && (strcmp(argv[2], "-d") == 0)) debug_enabled = true;
    if((argc == 3) && (strcmp(argv[2], "-l") == 0)) listing_enabled = true;

    // Init tables
    initGlobalLabelTable();
    initLocalLabelTable();
    initAnonymousLabelTable();

    // Assemble input to output
    gettimeofday(&start, NULL);
    assemble(infile, outfile);
    gettimeofday(&stop, NULL);
    printf("Assembly took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

    if(global_errors) printf("Error in input\n");

    closefiles();   
    return 0;
}