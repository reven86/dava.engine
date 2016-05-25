#ifndef __STATIC_OCCLUSION_TOOL_H__
#define __STATIC_OCCLUSION_TOOL_H__

#include "CommandLine/CommandLineTool.h"
class StaticOcclusionTool : public CommandLineTool
{
    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,
        ACTION_BUILD,
    };

public:
    StaticOcclusionTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;
    DAVA::FilePath GetQualityConfigPath() const override;

    eAction commandAction = ACTION_NONE;
    DAVA::FilePath scenePathname;
    DAVA::FilePath qualityConfigPath;
};


#endif // __STATIC_OCCLUSION_TOOL_H__
