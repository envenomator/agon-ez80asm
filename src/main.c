#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "utils.h"
#include "assemble.h"
#include "label.h"
#include "./stdint.h"
#include "macro.h"
#include "mos-interface.h"
#include "mos_posix.h"
#include "malloc.h"
#include "io.h"

int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("ez80asm version %s\r\n\r\n",VERSION);
        printf("Usage: asm <filename> [-l]\n\r");
        return 2;
    }
    list_enabled = ((argc == 3) && (strcmp(argv[2], "-l") == 0));
    consolelist_enabled = false;

    mos_posix_init();       // Init posix compatibility for non-MOS builds, before io_init
    if(!io_init(argv[1])) {
        printf("Error opening \"%s\"\r\n", argv[1]);
        return 2;
    }

    // Initialization
    initGlobalLabelTable();
    initLocalLabelTable();
    initAnonymousLabelTable();
    initMacros();
    init_agon_malloc();
    
    // Assemble input to output
    assemble();
    if(global_errors) printf("Error in input\r\n");
    else printf("Done\r\n");
    io_close();

    if(global_errors) return 1;
    return 0;
}
