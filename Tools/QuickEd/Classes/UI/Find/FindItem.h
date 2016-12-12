#pragma once

#include "FileSystem/FilePath.h"

class FindItem
{
public:
    FindItem(const DAVA::FilePath& file, const DAVA::String& pathToControl);
    ~FindItem();

    const DAVA::FilePath& GetFile() const;
    const DAVA::String& GetPathToControl() const;

    bool operator<(const FindItem& other) const;

private:
    DAVA::FilePath file;
    DAVA::String pathToControl;
};
