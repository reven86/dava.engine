#pragma once

#include "FileSystem/FilePath.h"
#include "Base/Singleton.h"

/**
singleton class responsible at reading some project-related resources.
It is based on ProjectManager class, but excluding particles reloading, signals etc.
Probably will be refactored/removed after implementing of resource system
*/
class ConsoleProject : public DAVA::Singleton<ConsoleProject>
{
public:
    /**
    destructor. closes project
    */
    ~ConsoleProject();

    /**
    adds project Data folder to resources 
    and loads some config and information yamls into corresponding singletons
    */
    void OpenProject(const DAVA::FilePath& path);
    /**
    removes project Data folder from resources
    */
    void CloseProject();

private:
    DAVA::FilePath projectPath;
};
