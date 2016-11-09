#pragma once

#include "Beast/BeastProxy.h"

#if defined(__DAVAENGINE_BEAST__)

class SceneEditor2;
class BeastManager;
class QtWaitDialog;

class BeastRunner final
{
public:
    BeastRunner(SceneEditor2* scene, const DAVA::FilePath& outputPath, BeastProxy::eBeastMode mode, QtWaitDialog* _waitDialog);
    ~BeastRunner();

    void RunUIMode();

    void Start();
    bool Process();
    void Finish(bool canceled);

private:
    void PackLightmaps();
    DAVA::FilePath GetLightmapDirectoryPath();

private:
    BeastManager* beastManager = nullptr;
    QtWaitDialog* waitDialog = nullptr;
    SceneEditor2* workingScene = nullptr;
    DAVA::FilePath outputPath;
    DAVA::uint64 startTime = 0;
    BeastProxy::eBeastMode beastMode = BeastProxy::eBeastMode::MODE_LIGHTMAPS;

    bool cancelledManually = false;
};

#endif //#if defined (__DAVAENGINE_BEAST__)
