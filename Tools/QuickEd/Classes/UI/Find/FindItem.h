#pragma once

#include "FileSystem/FilePath.h"

class FindItem
{
public:
    FindItem(const DAVA::FilePath& file, const DAVA::String& pathToControl, bool inPrototypeSection);
    ~FindItem();

private:
    DAVA::FilePath file;
    DAVA::String pathToControl;
    bool inPrototypeSection;
};
