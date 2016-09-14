#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

class ProjectStructure
{
public:
    ProjectStructure();
    ~ProjectStructure();

    void AddProjectDirectory(const DAVA::FilePath& directory);
    void ClearProjectDirectories();

    DAVA::Vector<DAVA::FilePath> GetFiles(const DAVA::String& extension) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
