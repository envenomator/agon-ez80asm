#ifndef CONFIG_H
#define CONFIG_H

#ifndef CEDEV
#include <stdint.h>
typedef int32_t int24_t;
typedef uint32_t uint24_t;
#endif

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#pragma warning(disable:4996)			// 'strcpy': This function or variable may be unsafe. Consider using strcpy_s instead
#pragma warning(disable:4267)			// conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable:4244)			// conversion from '__int64' to 'uint16_t', possible loss of data
#endif

#define VERSION                       1
#define REVISION                      8
#define ADLMODE_START              true
#define START_ADDRESS           0x40000 // Agon default load address
#define FILLBYTE                   0xFF // Same as ZDS
#define INSTRUCTION_HASHTABLESIZE   256 // Number of entries in the hashtable
#define GLOBAL_LABEL_TABLE_SIZE     256
#define FILESTACK_MAXFILES            4 // Maximum simultaneous include 'depth'
#define LINEMAX                     256
#define FILENAMEMAXLENGTH            64
#define FILES                         3 // Output files (binary / listing / anonymous labels)
#define FILE_BUFFERSIZE           32768 // For each specified output file (io.c)
#define MACRO_BUFFERSIZE		   2048 // maximum macro (ascii text) body size
#define LISTING_OBJECTS_PER_LINE      4 // Listing hex 'objects' between PC / Line number
#define TOKEN_MAX               LINEMAX // Token maximum length
#define MAXNAMELENGTH                32 // Maximum name length of labels
#define MACROMAXARGS                  8 // Maximum arguments to a macro
#define MACROARGLENGTH               32 // Maximum length of macro argument
#define CLEANUPFILES               true
#endif // CONFIG_H
