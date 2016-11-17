#pragma once

#include "Beast/BeastProxy.h"

#if defined(__DAVAENGINE_BEAST__)

namespace DAVA
{
class Scene;
}

class BeastManager;
class QtWaitDialog;

class BeastRunner final
{
public:
    BeastRunner(DAVA::Scene* scene, const DAVA::FilePath& scenePath, const DAVA::FilePath& outputPath, BeastProxy::eBeastMode mode, QtWaitDialog* _waitDialog);
    ~BeastRunner();

    void RunUIMode();

    void Start();
    bool Process();
    void Finish(bool canceled);

private:
    void PackLightmaps();
    DAVA::FilePath GetLightmapDirectoryPath() const;

private:
    BeastManager* beastManager = nullptr;
    QtWaitDialog* waitDialog = nullptr;
    DAVA::Scene* workingScene = nullptr;
    const DAVA::FilePath scenePath;
    DAVA::FilePath outputPath;
    DAVA::uint64 startTime = 0;
    BeastProxy::eBeastMode beastMode = BeastProxy::eBeastMode::MODE_LIGHTMAPS;

    bool cancelledManually = false;
};

#endif //#if defined (__DAVAENGINE_BEAST__)
