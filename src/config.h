#ifndef CONFIG_H
#define CONFIG_H

#define AGON

#define ADLMODE_START              true
#define START_ADDRESS           0x40000
#define FILLBYTE                   0x00 // NOP
#define FILESTACK_MAXFILES            4 // Maximum simultaneous include 'depth'
#define LINEMAX                     128 // Input line length maximum
#define FILENAMEMAXLENGTH            64
#define FILES                       6+1 // +current
#define GLOBAL_LABEL_BUFFERSIZE 0x40000 // 256KB global label space
#define LOCAL_LABEL_BUFFERSIZE   0x4000 // 16KB local label space
#define GLOBAL_LABEL_TABLE_SIZE   16384 // 16K global labels
#define LOCAL_LABEL_TABLE_SIZE       64 //  64 local labels
#define LISTING_OBJECTS_PER_LINE      6 // Listing hex 'objects' between PC / Line number
#define TOKEN_MAX                    64 // Token maximum length 
#define MAXIMUM_MACROS               64 // Maximum number of macros
#define MACRO_BUFFERSIZE         0x4000 // 16KB local macro space

#endif // CONFIG_H