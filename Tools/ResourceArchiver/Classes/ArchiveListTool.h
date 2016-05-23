#ifndef __ARCHIVE_LIST_TOOL_H__
#define __ARCHIVE_LIST_TOOL_H__

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


#endif // __ARCHIVE_LIST_TOOL_H__
