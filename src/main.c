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

    closeAllFiles();   
    return 0;
}