#pragma once

#include "Base/BaseTypes.h"
#include "Base/ScopedPtr.h"
#include "FileSystem/FilePath.h"
#include "CommandLine/Private/REConsoleModuleCommon.h"

class SceneEditor2;
class StaticOcclusionTool : public REConsoleModuleCommon
{
    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,
        ACTION_BUILD,
    };

public:
    StaticOcclusionTool(const DAVA::Vector<DAVA::String>& commandLine);

protected:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    DAVA::ScopedPtr<SceneEditor2> scene;
    eAction commandAction = ACTION_NONE;
};
