#pragma once

#include "TArc/DataProcessing/DataNode.h"
#include "FileSystem/FilePath.h"

class LibraryWidget;
class LibraryData : public DAVA::TArc::DataNode
{
public:
    static const char* selectedPathProperty;
    const DAVA::FilePath& GetSelectedPath() const;

    LibraryWidget* libraryWidget = nullptr;

private:
    DAVA_VIRTUAL_REFLECTION(LibraryData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<LibraryData>::Begin()
        .Field(selectedPathProperty, &LibraryData::GetSelectedPath, nullptr)
        .End();
    }
};
