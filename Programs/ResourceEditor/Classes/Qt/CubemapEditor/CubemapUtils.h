#ifndef __CUBEMAP_UTILS_H__
#define __CUBEMAP_UTILS_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

class CubemapUtils
{
public:
    static void GenerateFaceNames(const DAVA::String& baseName, DAVA::Vector<DAVA::FilePath>& faceNames);
    static DAVA::FilePath GetDialogSavedPath(const DAVA::String& key, const DAVA::String& defaultValue);
};

#endif /* defined(__CUBEMAP_UTILS_H__) */
