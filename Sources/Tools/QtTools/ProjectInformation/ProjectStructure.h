#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

class ProjectStructure
{
public:
    ProjectStructure(const DAVA::Vector<DAVA::String>& supportedExtensions);
    ~ProjectStructure();

    void SetProjectDirectory(const DAVA::FilePath& directory);
    DAVA::FilePath GetProjectDirectory() const;

    DAVA::Vector<DAVA::FilePath> GetFiles(const DAVA::String& extension) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
