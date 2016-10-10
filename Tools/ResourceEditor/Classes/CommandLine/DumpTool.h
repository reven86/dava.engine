#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"

class DumpTool : public REConsoleModuleCommon
{
    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,
        ACTION_DUMP_LINKS
    };

public:
    DumpTool();

protected:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;

private:
    bool ReadCommandLine();

    eAction commandAction = ACTION_NONE;
    DAVA::String filename;
    DAVA::FilePath inFolder;
    DAVA::FilePath outFile;
    DAVA::FilePath qualityPathname;
};
