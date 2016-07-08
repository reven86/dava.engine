#ifndef __IMAGE_SPLITTER_TOOL_H__
#define __IMAGE_SPLITTER_TOOL_H__

#include "CommandLine/CommandLineTool.h"

class ImageSplitterTool : public CommandLineTool
{
    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,

        ACTION_SPLIT,
        ACTION_MERGE
    };

public:
    ImageSplitterTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;

    eAction commandAction;
    DAVA::FilePath filename;
    DAVA::FilePath foldername;
};


#endif // __IMAGE_SPLITTER_TOOL_H__
