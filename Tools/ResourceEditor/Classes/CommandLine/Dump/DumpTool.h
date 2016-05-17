#ifndef __DUMP_TOOL_H__
#define __DUMP_TOOL_H__

#include "CommandLine/CommandLineTool.h"

class DumpTool : public CommandLineTool
{
    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,
        ACTION_DUMP_LINKS,
    };

public:
    DumpTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;
    DAVA::FilePath GetQualityConfigPath() const override;

    eAction commandAction = ACTION_NONE;
    DAVA::String filename;
    DAVA::FilePath inFolder;
    DAVA::FilePath outFile;
    DAVA::FilePath qualityPathname;
};


#endif // __DUMP_TOOL_H__
