#include "Classes/Library/Private/LIbraryData.h"

const char* LibraryData::selectedPathProperty = "SelectedPath";

const DAVA::FilePath& LibraryData::GetSelectedPath() const
{
    return selectedPath;
}
