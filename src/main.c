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
#include <getopt.h>
#include "str2num.h"

void printVersion(void) {
    printf("ez80asm version %d.%d, (C)2023 - Jeroen Venema\r\n",VERSION,REVISION);
}

void printHelp(void) {
    printf("Usage: ez80asm <filename> [OPTION]\n\r\r\n");
    printf("  -v\tList version information\r\n");
    printf("  -h\tList help information\r\n");
    printf("  -o\tOrg start address in hexadecimal format, default is %06X\r\n", START_ADDRESS);
    printf("  -b\tFillbyte in hexadecimal format, default is %02X\r\n", FILLBYTE);
    printf("  -a\tADL mode 1/0, default is %d\r\n", ADLMODE_START);
    printf("  -l\tListing to file with .lst extension\r\n");
    printf("  -d\tDirect listing to console\r\n");
    printf("\r\n");
}

int main(int argc, char *argv[])
{
    int opt;
    char inputfilename[FILENAMEMAXLENGTH + 1];
    int filenamecount = 0;

    // option defaults from compiled configuration
    fillbyte_start = FILLBYTE;
    list_enabled = false;
    adlmode_start = ADLMODE_START;
    start_address = START_ADDRESS;

    while ((opt = getopt(argc, argv, "-:ldvhb:a:o:")) != -1) {
        switch(opt) {
            case 'a':
                if((strlen(optarg) != 1) || 
                   ((*optarg != '0') && (*optarg != '1'))) {
                    error("Incorrect ADL mode option -a");
                    return 2;
                }
                adlmode_start = (*optarg == '1')?true:false;
                printf("Setting ADL mode to %d\r\n",adlmode_start);
                break;
            case 'd':
                //printf("Listing to console\r\n");
                consolelist_enabled = true;
                break;
            case 'l':
                list_enabled = true;
                break;
            case 'v':
                printVersion();
                break;
            case 'h':
                printHelp();
                return 0;
            case 'b':
                if(strlen(optarg) > 2) {
                    error("option -b: Byte range error");
                    return 2;
                }
                fillbyte_start = str2hex(optarg);
                if(err_str2num) {
                    error("option -b: Invalid hexadecimal format");
                    return 2;
                }
                printf("Setting fillbyte to hex %02X\r\n", fillbyte_start);
                break;
            case 'o':
                if(strlen(optarg) > 6) {
                    error("option -o: Address longer than 24bit");
                    return 2;
                }
                start_address = str2hex(optarg);
                if(err_str2num) {
                    error("option -o: Invalid hexadecimal format");
                    return 2;
                }
                printf("Setting org address to hex %06X\r\n", start_address);
                break;
            case '?':
                text_RED();
                printf("Missing argument or unknown option: %c\r\n", optopt);
                text_NORMAL();
                return 2;
            case 1:
                filenamecount++;
                strncpy(inputfilename, optarg, FILENAMEMAXLENGTH);
                break;
        }
    }
    if((argc == 1) || (filenamecount == 0)) {
        error("No input filename");
        printHelp();
        return 2;
    }
    if(filenamecount > 1) {
        error("Too many files provided as argument");
        return 2;
    }

    mos_posix_init();       // Init posix compatibility for non-MOS builds, before io_init

    if(!io_init(inputfilename)) {
        text_RED();
        printf("Error opening \"%s\"\r\n", inputfilename);
        text_NORMAL();
        return 2;
    }
    printf("Assembling %s\r\n", inputfilename);
    if(list_enabled) printf("Listing to %s\r\n", filename[FILE_LISTING]);

    // Initialization
    initGlobalLabelTable();
    initLocalLabelTable();
    initAnonymousLabelTable();
    initMacros();
    init_agon_malloc();
    
    // Assemble input to output
    assemble();
    if(!global_errors) printf("Done\r\n");
    io_close();

    if(global_errors) return 1;
    return 0;
}
