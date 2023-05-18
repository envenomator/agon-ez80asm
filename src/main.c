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
    printf("ez80asm version %s, (C)2023 - Jeroen Venema\r\n",VERSION);
}

void printHelp(void) {
    printf("Usage: ez80asm <filename> [OPTION]\n\r\r\n");
    printf("  -v\tList version information\r\n");
    printf("  -h\tList help information\r\n");
    printf("  -x\tStart address in hexadecimal format, default is 0x%06X\r\n", START_ADDRESS);
    printf("  -b\tFillbyte in hexadecimal format xHH/0xHH, default is 0x%02X\r\n", FILLBYTE);
    printf("  -a\tADL mode 1/0, default is %d\r\n", ADLMODE_START);
    printf("  -l\tListing to file with .lst extension\r\n");
    printf("  -d\tDirect listing to console\r\n");
    printf("\r\n");
}

int main(int argc, char *argv[])
{
    int opt;
    int index;
    char filename[FILENAMEMAXLENGTH];
    int filenamecount = 0;

    // option defaults from compiled configuration
    fillbyte = FILLBYTE;
    list_enabled = false;
    adlmode_start = ADLMODE_START;
    start_address = START_ADDRESS;

    while ((opt = getopt(argc, argv, "-:ldvhb:a:x:")) != -1) {
        switch(opt) {
            case 'a':
                if((strlen(optarg) != 1) || 
                   ((*optarg != '0') && (*optarg != '1'))) {
                    printf("Incorrect ADL mode option -a\r\n");
                    return 2;
                }
                adlmode_start = (*optarg == '1')?true:false;
                //printf("ADL mode start is %d\r\n",adlmode_start);
                break;
            case 'd':
                //printf("Listing to console\r\n");
                consolelist_enabled = true;
                break;
            case 'l':
                //printf("Listing to file\r\n");
                list_enabled = true;
                break;
            case 'v':
                printVersion();
                return 0;
            case 'h':
                printVersion();
                printHelp();
                return 0;
            case 'b':
                index = 0;
                if((strncmp(optarg, "x", 1) == 0) && strlen(optarg) < 4) index = 1;
                if((strncmp(optarg, "0x", 2) == 0) && strlen(optarg) < 5) index = 2;
                if(index) {
                    fillbyte = str2hex(optarg + index);
                    if(!err_str2num) {
                        //printf("Setting fillbyte to 0x%02x\r\n", fillbyte);
                        break;
                    }
                }
                printf("Invalid format for option -b\r\n");
                return 2;
            case 'x':
                index = 0;
                if((strncmp(optarg, "x", 1) == 0) && strlen(optarg) < 8) index = 1;
                if((strncmp(optarg, "0x", 2) == 0) && strlen(optarg) < 9) index = 2;
                if(index) {
                    start_address = str2hex(optarg + index);
                    if(!err_str2num) {
                        //printf("Setting address to 0x%06X\r\n", start_address);
                        break;
                    }
                }
                printf("Invalid format for option -x\r\n");
                return 2;
            case '?':
                printf("Missing argument or unknown option: %c\r\n", optopt);
                return 2;
            case 1:
                filenamecount++;
                strncpy(filename, optarg, FILENAMEMAXLENGTH);
                break;
        }
    }
    if((argc == 1) || (filenamecount == 0)) {
        printVersion();
        printHelp();
        return 2;
    }
    if(filenamecount > 1) {
        printf("Too many files provided as argument\r\n");
        return 2;
    }

    mos_posix_init();       // Init posix compatibility for non-MOS builds, before io_init

    if(!io_init(filename)) {
        printf("Error opening \"%s\"\r\n", filename);
        return 2;
    }

    printf("Assembling %s\r\n", filename);
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
