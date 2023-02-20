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

char filename_bin[FILENAMEMAXLENGTH];
char filename_locals[FILENAMEMAXLENGTH];
char filename_anon[FILENAMEMAXLENGTH];
char filename_list[FILENAMEMAXLENGTH];

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
    if(file_list) fclose(file_list);
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
    strcpy(filename_list,filename_bin);
    strcat(filename_bin, ".bin");
    strcat(filename_locals, ".locallabels");
    strcat(filename_anon, ".anonlabels");
    strcat(filename_list, ".lst");

    status = status && openFile(&file_input, basename, "r");
    status = status && openFile(&file_bin, filename_bin, "wb+");
    status = status && openFile(&file_locals, filename_locals, "wb+");
    status = status && openFile(&file_anon, filename_anon, "wb+");
    status = status && openFile(&file_list, filename_list, "wb+");
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

    if(!openfiles(argv[1])) return 0;

    debug_enabled = false;
    if((argc == 3) && (strcmp(argv[2], "-d") == 0)) debug_enabled = true;
    if((argc == 3) && (strcmp(argv[2], "-l") == 0)) consolelist_enabled = true;

    // Init tables
    initGlobalLabelTable();
    initLocalLabelTable();
    initAnonymousLabelTable();

    // Assemble input to output
    gettimeofday(&start, NULL);
    assemble(file_input, argv[1]);
    gettimeofday(&stop, NULL);
    if(global_errors) {
        remove(filename_bin);
        printf("Error in input\n");
    }
    else printf("%d bytes\n", totalsize);
    printf("\nAssembly took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

    closeFiles();   
    return 0;
}