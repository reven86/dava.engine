#include "Classes/Library/Private/LibraryData.h"
#include "Classes/Library/Private/LibraryWidget.h"

const char* LibraryData::selectedPathProperty = "SelectedPath";

const DAVA::FilePath& LibraryData::GetSelectedPath() const
{
    DVASSERT(libraryWidget != nullptr);
    return libraryWidget->GetSelectedPath();
}
