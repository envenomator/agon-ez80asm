#include "filestack.h"

filestackitem _filestack[FILESTACK_MAXFILES + 1];
uint8_t       _filestackCount;

void filestackInit(void) {
    _filestackCount = 0;
}

uint8_t filestackCount(void) {
    return _filestackCount;
}

bool filestackPush(filestackitem *fs) {
    if(_filestackCount < FILESTACK_MAXFILES) {
        _filestack[_filestackCount] = *fs;
        _filestackCount++;
        return true;
    }
    else error(message[ERROR_MAXINCLUDEFILES]);
    return false;
}

bool filestackPop(filestackitem *fs) {
    if(_filestackCount) {
        _filestackCount--;
        *fs = _filestack[_filestackCount];
        return true;
    }
    return false;
}
