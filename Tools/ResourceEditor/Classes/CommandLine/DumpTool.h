#pragma once

#include "Base/BaseTypes.h"
#include "CommandLine/Private/REConsoleModuleCommon.h"

class DumpTool : public REConsoleModuleCommon
{
public:
    DumpTool(const DAVA::Vector<DAVA::String>& commandLine);

private:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,
        ACTION_DUMP_LINKS
    };
    eAction commandAction = ACTION_NONE;
    DAVA::String filename;
    DAVA::FilePath inFolder;
    DAVA::FilePath outFile;
};
