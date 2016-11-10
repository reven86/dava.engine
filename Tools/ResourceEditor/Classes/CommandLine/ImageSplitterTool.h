#pragma once

#include "CommandLine/CommandLineModule.h"

class ImageSplitterTool : public CommandLineModule
{
public:
    ImageSplitterTool(const DAVA::Vector<DAVA::String>& commandLine);

    static const DAVA::String Key;

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
