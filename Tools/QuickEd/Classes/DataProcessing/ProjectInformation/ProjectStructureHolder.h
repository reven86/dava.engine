#pragma once

#include "Base/BaseTypes.h"

class ProjectStructureHolder
{
public:
    ProjectStructureHolder();
    ~ProjectStructureHolder();

    void SetProjectDirectory(const DAVA::String& directory);

    DAVA::Vector<DAVA::String> GetProjectYamlFiles() const;

    DAVA::String GetProjectDirectory() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
