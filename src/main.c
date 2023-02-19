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

char filename_bin[FILENAMEMAXLENGTH];
char filename_locals[FILENAMEMAXLENGTH];
char filename_anon[FILENAMEMAXLENGTH];

bool openFile(FILE **file, char *name, char *mode) {
    *file = fopen(name, mode);

    if(*file) return true;
    printf("Error opening \"%s\"\n", name);
    return false;
}

void closeFiles() {
   // Cleanup
    if(file_input) fclose(file_input);
    if(file_bin) fclose(file_bin);
    if(file_locals) fclose(file_locals);
    if(file_anon) fclose(file_anon);
    remove(filename_locals);
    remove(filename_anon);
}

bool openfiles(char *basename) {
    bool status = true;

    // prepare filenames
    strcpy(filename_bin, basename);
    remove_ext(filename_bin, '.', '/');
    strcpy(filename_locals, filename_bin);
    strcpy(filename_anon,filename_bin);
    strcat(filename_bin, ".bin");
    strcat(filename_locals, ".lbls");
    strcat(filename_anon, ".anolbls");

    status = status && openFile(&file_input, basename, "r");
    status = status && openFile(&file_bin, filename_bin, "wb+");
    status = status && openFile(&file_locals, filename_locals, "wb+");
    status = status && openFile(&file_anon, filename_anon, "wb+");

    if(!status) closeFiles();
    return status;
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
    if((argc == 3) && (strcmp(argv[2], "-d") == 0)) debug_enabled = true;

    // Init tables
    initGlobalLabelTable();
    initLocalLabelTable();
    initAnonymousLabelTable();

    // Assemble input to output
    gettimeofday(&start, NULL);
    assemble(file_input, file_bin);
    gettimeofday(&stop, NULL);
    printf("\nAssembly took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

    if(global_errors) printf("Error in input\n");

    closeFiles();   
    return 0;
}