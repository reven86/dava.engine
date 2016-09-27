#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

class ResaveUtility
{
public:
    void InitFromCommandLine();
    void Resave();

private:
    DAVA::Vector<DAVA::FilePath> filesToResave;
};