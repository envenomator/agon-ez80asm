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

//char filename_bin[FILENAMEMAXLENGTH];
//char filename_locals[FILENAMEMAXLENGTH];
//char filename_anon[FILENAMEMAXLENGTH];
//char filename_list[FILENAMEMAXLENGTH];


bool openFile(FILE **file, char *name, char *mode) {
    *file = fopen(name, mode);

    if(*file) return true;
    printf("Error opening \"%s\"\n", name);
    return false;
}

void prepare_filenames(char *input_filename) {
    // prepare filenames
    strcpy(filename[FILE_INPUT], input_filename);
    strcpy(filename[FILE_OUTPUT], input_filename);
    remove_ext(filename[FILE_OUTPUT], '.', '/');
    strcpy(filename[FILE_LOCAL_LABELS], filename[FILE_OUTPUT]);
    strcpy(filename[FILE_ANONYMOUS_LABELS],filename[FILE_OUTPUT]);
    strcpy(filename[FILE_LISTING],filename[FILE_OUTPUT]);
    strcat(filename[FILE_OUTPUT], ".bin");
    strcat(filename[FILE_LOCAL_LABELS], ".locallabels");
    strcat(filename[FILE_ANONYMOUS_LABELS], ".anonlabels");
    strcat(filename[FILE_LISTING], ".lst");
}

void closeFiles() {
   // Cleanup
    if(filehandle[FILE_INPUT]) fclose(filehandle[FILE_INPUT]);
    if(filehandle[FILE_OUTPUT]) fclose(filehandle[FILE_OUTPUT]);
    if(filehandle[FILE_LOCAL_LABELS]) fclose(filehandle[FILE_LOCAL_LABELS]);
    if(filehandle[FILE_ANONYMOUS_LABELS]) fclose(filehandle[FILE_ANONYMOUS_LABELS]);
    if(filehandle[FILE_LISTING]) fclose(filehandle[FILE_LISTING]);
    remove(filename[FILE_LOCAL_LABELS]);
    remove(filename[FILE_ANONYMOUS_LABELS]);
}

bool openfiles(void) {
    bool status = true;

    status = status && openFile(&filehandle[FILE_INPUT], filename[FILE_INPUT], "r");
    status = status && openFile(&filehandle[FILE_OUTPUT], filename[FILE_OUTPUT], "wb+");
    status = status && openFile(&filehandle[FILE_LOCAL_LABELS], filename[FILE_LOCAL_LABELS], "wb+");
    status = status && openFile(&filehandle[FILE_ANONYMOUS_LABELS], filename[FILE_ANONYMOUS_LABELS], "wb+");
    status = status && openFile(&filehandle[FILE_LISTING], filename[FILE_LISTING], "wb+");
    if(!status) closeFiles();
    return status;
}

int main(int argc, char *argv[])
{
    struct timeval stop, start;

    if(argc < 2){
        printf("Usage: asm <filename> [-l]\n");
        exit(1);
    }

    prepare_filenames(argv[1]);
    if(!openfiles()) return 0;

    debug_enabled = false;
    if((argc == 3) && (strcmp(argv[2], "-d") == 0)) debug_enabled = true;
    if((argc == 3) && (strcmp(argv[2], "-l") == 0)) consolelist_enabled = true;

    // Init tables
    initGlobalLabelTable();
    initLocalLabelTable();
    initAnonymousLabelTable();

    // Assemble input to output
    gettimeofday(&start, NULL);
    assemble();
    gettimeofday(&stop, NULL);
    if(global_errors) {
        remove(filename[FILE_OUTPUT]);
        printf("Error in input\n");
    }
    else printf("%d bytes\n", totalsize);
    printf("\nAssembly took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

    closeFiles();   
    return 0;
}