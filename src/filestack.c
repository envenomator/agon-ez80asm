#include "filestack.h"
#include "utils.h"

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
        _filestack[_filestackCount].address = fs->address;
        _filestack[_filestackCount].linenumber = fs->linenumber;
        _filestack[_filestackCount].fp = fs->fp;
        _filestackCount++;
        return true;
    }
    else error("Maximum include files reached");
    return false;
}

bool filestackPop(filestackitem *fs) {
    if(_filestackCount) {
        _filestackCount--;
        fs->address = _filestack[_filestackCount].address;
        fs->linenumber = _filestack[_filestackCount].linenumber;
        fs->fp = _filestack[_filestackCount].fp;
        return true;
    }
    return false;
}
