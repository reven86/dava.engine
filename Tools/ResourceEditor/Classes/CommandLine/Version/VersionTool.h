#ifndef VERSION_TOOL_H
#define VERSION_TOOL_H

#include "CommandLine/CommandLineTool.h"

class VersionTool : public CommandLineTool
{
public:
    VersionTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;
};


#endif // VERSION_TOOL_H
