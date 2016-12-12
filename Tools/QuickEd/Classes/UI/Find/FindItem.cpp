#include "FindItem.h"

FindItem::FindItem(const DAVA::FilePath& file_, const DAVA::String& pathToControl_)
    : file(file_)
    , pathToControl(pathToControl_)
{
}

FindItem::~FindItem()
{
}

const DAVA::FilePath& FindItem::GetFile() const
{
    return file;
}

const DAVA::String& FindItem::GetPathToControl() const
{
    return pathToControl;
}

bool FindItem::operator<(const FindItem& other) const
{
    if (file.GetFrameworkPath() == other.GetFile().GetFrameworkPath())
    {
        return pathToControl < other.pathToControl;
    }
    return file.GetFrameworkPath() < other.GetFile().GetFrameworkPath();
}
