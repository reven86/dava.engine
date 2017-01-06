#include "BeastRunner.h"

#if defined(__DAVAENGINE_BEAST__)

#include "Scene3D/Scene.h"

#include "Main/mainwindow.h"
#include "Beast/BeastProxy.h"
#include "Beast/LightmapsPacker.h"
#include "Settings/SettingsManager.h"
#include "Utils/SceneUtils/SceneUtils.h"

#include "DAVAEngine.h"


#include "SceneParser.h"

//Beast
BeastRunner::BeastRunner(DAVA::Scene* scene, const DAVA::FilePath& scenePath_, const DAVA::FilePath& outputPath_, BeastProxy::eBeastMode mode, QtWaitDialog* waitDialog_)
    : workingScene(scene)
    , scenePath(scenePath_)
    , waitDialog(waitDialog_)
    , outputPath(outputPath_)
    , beastMode(mode)
{
    outputPath.MakeDirectoryPathname();
    beastManager = BeastProxy::Instance()->CreateManager();
    BeastProxy::Instance()->SetMode(beastManager, mode);
}

BeastRunner::~BeastRunner()
{
    BeastProxy::Instance()->SafeDeleteManager(&beastManager);
}

void BeastRunner::RunUIMode()
{
    if (waitDialog != nullptr)
    {
        waitDialog->Show("Beast process", "Starting Beast", true, true);
        waitDialog->SetRange(0, 100);
        waitDialog->EnableCancel(false);
    }

    Start();

    while (Process() == false)
    {
        if (cancelledManually)
        {
            BeastProxy::Instance()->Cancel(beastManager);
            break;
        }
        else if (waitDialog != nullptr)
        {
            waitDialog->SetValue(BeastProxy::Instance()->GetCurTaskProcess(beastManager));
            waitDialog->SetMessage(BeastProxy::Instance()->GetCurTaskName(beastManager).c_str());
            waitDialog->EnableCancel(true);
            cancelledManually |= waitDialog->WasCanceled();
        }

        Sleep(15);
    }

    if (waitDialog != nullptr)
    {
        waitDialog->EnableCancel(false);
    }

    Finish(cancelledManually || BeastProxy::Instance()->WasCancelled(beastManager));

    if (waitDialog != nullptr)
    {
        waitDialog->Reset();
    }
}

void BeastRunner::Start()
{
    startTime = DAVA::SystemTimer::GetMs();

    DAVA::FilePath path = GetLightmapDirectoryPath();
    if (beastMode == BeastProxy::MODE_LIGHTMAPS)
    {
        DAVA::FileSystem::Instance()->CreateDirectory(path, false);
        DAVA::FileSystem::Instance()->CreateDirectory(outputPath, true);
    }

    BeastProxy::Instance()->SetLightmapsDirectory(beastManager, path);
    BeastProxy::Instance()->Run(beastManager, workingScene);
}

bool BeastRunner::Process()
{
    BeastProxy::Instance()->Update(beastManager);
    return BeastProxy::Instance()->IsJobDone(beastManager);
}

void BeastRunner::Finish(bool canceled)
{
    if (!canceled && beastMode == BeastProxy::MODE_LIGHTMAPS)
    {
        PackLightmaps();
    }

    DAVA::FileSystem::Instance()->DeleteDirectory(SceneParser::GetTemporaryFolder());
    if (beastMode == BeastProxy::MODE_LIGHTMAPS)
    {
        DAVA::FileSystem::Instance()->DeleteDirectory(GetLightmapDirectoryPath());
    }
}

void BeastRunner::PackLightmaps()
{
    DAVA::FilePath inputDir = GetLightmapDirectoryPath();
    DAVA::FilePath outputDir = outputPath;

    DAVA::FileSystem::Instance()->MoveFile(inputDir + "landscape.png", scenePath.GetDirectory() + "temp_landscape_lightmap.png", true);

    LightmapsPacker packer;
    packer.SetInputDir(inputDir);

    packer.SetOutputDir(outputDir);
    packer.PackLightmaps(DAVA::GPU_ORIGIN);
    packer.CreateDescriptors();
    packer.ParseSpriteDescriptors();

    BeastProxy::Instance()->UpdateAtlas(beastManager, packer.GetAtlasingData());

    DAVA::FileSystem::Instance()->MoveFile(scenePath.GetDirectory() + "temp_landscape_lightmap.png", outputDir + "landscape.png", true);
    DAVA::FileSystem::Instance()->DeleteDirectory(scenePath.GetDirectory() + "$process/");
}

DAVA::FilePath BeastRunner::GetLightmapDirectoryPath() const
{
    return scenePath + "_beast/";
}


#endif //#if defined (__DAVAENGINE_BEAST__)
