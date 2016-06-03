#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/ResourceArchive.h"
#include "CommandLineTool.h"

class ArchiveListTool : public CommandLineTool
{
public:
    ArchiveListTool();

private:
    bool ConvertOptionsToParamsInternal() override;
    int ProcessInternal() override;

    DAVA::FilePath outFilePath;
    DAVA::FilePath packFilePath;
};
