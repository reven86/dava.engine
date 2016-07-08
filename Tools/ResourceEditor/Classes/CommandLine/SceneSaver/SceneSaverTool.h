#ifndef __SCENE_SAVER_TOOL_H__
#define __SCENE_SAVER_TOOL_H__

#include "CommandLine/CommandLineTool.h"

class SceneSaverTool : public CommandLineTool
{
    enum eAction : DAVA::int32
    {
        ACTION_NONE = 0,

        ACTION_SAVE,
        ACTION_RESAVE_SCENE,
        ACTION_RESAVE_YAML,
    };

public:
    SceneSaverTool();

private:
    void ConvertOptionsToParamsInternal() override;
    bool InitializeInternal() override;
    void ProcessInternal() override;
    DAVA::FilePath GetQualityConfigPath() const override;

    eAction commandAction = ACTION_NONE;
    DAVA::String filename;

    DAVA::FilePath inFolder;
    DAVA::FilePath outFolder;
    DAVA::FilePath qualityConfigPath;

    bool copyConverted = false;
};


#endif // __SCENE_SAVER_TOOL_H__
