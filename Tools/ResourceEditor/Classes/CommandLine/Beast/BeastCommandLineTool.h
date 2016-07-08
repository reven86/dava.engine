#ifndef __BEAST_COMMAND_LINE_TOOL_H__
#define __BEAST_COMMAND_LINE_TOOL_H__

#include "CommandLine/CommandLineTool.h"

#if defined(__DAVAENGINE_BEAST__)

class BeastCommandLineTool : public CommandLineTool
{
public:
    BeastCommandLineTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;

    DAVA::FilePath GetQualityConfigPath() const override;

    DAVA::FilePath scenePathname;
    DAVA::FilePath outputPath;
    DAVA::FilePath qualityConfigPath;
};

#endif //#if defined (__DAVAENGINE_BEAST__)

#endif // __BEAST_COMMAND_LINE_TOOL_H__
