#pragma once
#include "Base/BaseTypes.h"

namespace DAVA
{
struct IOErrorTypes
{
    int32 ioErrorCode = 0;
    bool openFile = false;
    bool createFile = false;
    bool writeFile = false;
    bool readFile = false;
    bool closeFile = false;
};
void GenerateIOErrorOnNextOperation(IOErrorTypes types);
bool GenErrorOnOpenFile();
bool GenErrorOnWriteToFile();

} // end namespace DAVA
