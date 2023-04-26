#ifndef CONFIG_H
#define CONFIG_H

#ifndef WINDOWS
#ifndef UNIX

#define AGON

#endif // UNIX
#endif // WINDOWS

#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#define ADLMODE_START              true
#define START_ADDRESS           0x40000 // Agon default load address
#define FILLBYTE                   0x00 // NOP
#define FILESTACK_MAXFILES            2 // Maximum simultaneous include 'depth'
#define LINEMAX                     128 // Input line length maximum
#define FILENAMEMAXLENGTH            64
#define FILES                         8
#define MALLOC_BUFFERSIZE       0x40000 // global 256KB memory buffer
#define LOCAL_LABEL_BUFFERSIZE   0x4000 // 16KB local label space
#define GLOBAL_LABEL_TABLE_SIZE   16384 // 16K global labels
#define LOCAL_LABEL_TABLE_SIZE       64 //  64 local labels
#define FILE_BUFFERSIZE            1024 // For each specified input/output file (io.c)
#define LISTING_OBJECTS_PER_LINE      6 // Listing hex 'objects' between PC / Line number
#define TOKEN_MAX                    64 // Token maximum length
#define MAXIMUM_MACROS               64 // Maximum number of macros
#define MACROMAXARGS                  8 // Maximum arguments to a macro
#define MACROARGLENGTH               32 // Maximum length of macro argument

#define CLEANUPFILES              true
#endif // CONFIG_H
