#ifndef CONFIG_H
#define CONFIG_H

#define START_ADDRESS   0x40000
#define FILLBYTE        0x00 // NOP
#define FILESTACK_MAXFILES  4
#define LINEMAX         128
#define FILENAMEMAXLENGTH 64
#define FILES               5+1 // +current
#define GLOBAL_LABEL_BUFFERSIZE 131072
#define LOCAL_LABEL_BUFFERSIZE    4096
#define GLOBAL_LABEL_TABLE_SIZE   8192
#define LOCAL_LABEL_TABLE_SIZE      64
#define OBJECTS_PER_LINE 6
#define TOKEN_MAX   128

#endif // CONFIG_H