#pragma once

#include "Base/ScopedPtr.h"
#include "FileSystem/FilePath.h"
#include "CommandLine/CommandLineModule.h"

namespace DAVA
{
class Scene;
class StaticOcclusionBuildSystem;
}

class StaticOcclusionTool : public CommandLineModule
{
public:
    StaticOcclusionTool(const DAVA::Vector<DAVA::String>& commandLine);

    static const DAVA::String Key;

protected:
    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void BeforeDestroyedInternal() override;
    void ShowHelpInternal() override;

    DAVA::FilePath scenePathname;
    DAVA::ScopedPtr<DAVA::Scene> scene;
    DAVA::StaticOcclusionBuildSystem* staticOcclusionBuildSystem = nullptr;

    enum eAction : DAVA::int32
    {
        ACTION_NONE = -1,
        ACTION_BUILD,
    };
    eAction commandAction = ACTION_NONE;
};
