#include <string.h>
#include "filestack.h"
#include "utils.h"
#include "./stdint.h"

filestackitem _filestack[FILESTACK_MAXFILES];
uint8_t       _filestackCount;

void filestackInit(void) {
    _filestackCount = 0;
}

uint8_t filestackCount(void) {
    return _filestackCount;
}

bool filestackPush(filestackitem *fs) {
    if(_filestackCount < FILESTACK_MAXFILES) {
        _filestack[_filestackCount].linenumber = fs->linenumber;
        _filestack[_filestackCount].fp = fs->fp;
        strcpy(_filestack[_filestackCount].filename,fs->filename);
        _filestackCount++;
        return true;
    }
    else error(message[ERROR_MAXINCLUDEFILES]);
    return false;
}

bool filestackPop(filestackitem *fs) {
    if(_filestackCount) {
        _filestackCount--;
        fs->linenumber = _filestack[_filestackCount].linenumber;
        fs->fp = _filestack[_filestackCount].fp;
        strcpy(fs->filename, _filestack[_filestackCount].filename);
        return true;
    }
    return false;
}
