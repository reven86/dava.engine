#pragma once

#include "CommandLine/Private/REConsoleModuleCommon.h"

class ImageSplitterTool : public REConsoleModuleCommon
{
public:
    ImageSplitterTool(const DAVA::Vector<DAVA::String>& commandLine);

protected:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void ShowHelpInternal() override;

    DAVA::FilePath filename;
    DAVA::FilePath foldername;

    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,

        ACTION_SPLIT,
        ACTION_MERGE
    };

    eAction commandAction = ACTION_NONE;
};
