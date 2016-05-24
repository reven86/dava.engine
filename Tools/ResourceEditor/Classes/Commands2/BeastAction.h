#ifndef __BEAST_ACTION_H__
#define __BEAST_ACTION_H__

#include "Commands2/Base/CommandAction.h"
#include "Beast/BeastProxy.h"

#if defined(__DAVAENGINE_BEAST__)

class SceneEditor2;
class BeastManager;
class QtWaitDialog;

class BeastAction : public CommandAction
{
public:
    BeastAction(SceneEditor2* scene, const DAVA::FilePath& outputPath, BeastProxy::eBeastMode mode, QtWaitDialog* _waitDialog);
    ~BeastAction();

    virtual void Redo();

private:
    void Start();
    bool Process();
    void Finish(bool canceled);

    void PackLightmaps();
    DAVA::FilePath GetLightmapDirectoryPath();

private:
    BeastManager* beastManager = nullptr;
    QtWaitDialog* waitDialog = nullptr;
    SceneEditor2* workingScene = nullptr;
    DAVA::FilePath outputPath;
    DAVA::uint64 startTime = 0;
    BeastProxy::eBeastMode beastMode = BeastProxy::eBeastMode::MODE_LIGHTMAPS;
};

#endif //#if defined (__DAVAENGINE_BEAST__)

#endif // #ifndef __BEAST_ACTION_H__