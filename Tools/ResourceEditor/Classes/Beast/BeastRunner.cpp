#include "BeastRunner.h"

#if defined(__DAVAENGINE_BEAST__)

#include "Scene/SceneEditor2.h"
#include "Main/mainwindow.h"
#include "Beast/BeastProxy.h"
#include "Beast/LightmapsPacker.h"
#include "Settings/SettingsManager.h"
#include "CommandLine/SceneUtils/SceneUtils.h"
#include "CommandLine/Private/SceneConsoleHelper.h"

#include "DAVAEngine.h"


#include "SceneParser.h"

using namespace DAVA;

//Beast
BeastRunner::BeastRunner(SceneEditor2* scene, const DAVA::FilePath& _outputPath, BeastProxy::eBeastMode mode, QtWaitDialog* _waitDialog)
    : workingScene(scene)
    , waitDialog(_waitDialog)
    , outputPath(_outputPath)
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

void BeastRunner::Run()
{
    bool cancelledManually = false;

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

        // if waitDialog is nullptr, it means that we are working in console mode
        if (waitDialog == nullptr)
        {
            SceneConsoleHelper::ReleaseRendering();
        }
        Sleep(15);
    }

    if (waitDialog != nullptr)
    {
        waitDialog->EnableCancel(false);
    }

    Finish(cancelledManually | BeastProxy::Instance()->WasCancelled(beastManager));

    if (waitDialog != nullptr)
    {
        waitDialog->Reset();
    }
}

void BeastRunner::Start()
{
    startTime = SystemTimer::Instance()->AbsoluteMS();

    FilePath path = GetLightmapDirectoryPath();
    if (beastMode == BeastProxy::MODE_LIGHTMAPS)
    {
        FileSystem::Instance()->CreateDirectory(path, false);
        FileSystem::Instance()->CreateDirectory(outputPath, true);
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

    FileSystem::Instance()->DeleteDirectory(SceneParser::GetTemporaryFolder());
    if (beastMode == BeastProxy::MODE_LIGHTMAPS)
    {
        FileSystem::Instance()->DeleteDirectory(GetLightmapDirectoryPath());
    }
}

void BeastRunner::PackLightmaps()
{
    FilePath scenePath = workingScene->GetScenePath();
    FilePath inputDir = GetLightmapDirectoryPath();
    FilePath outputDir = outputPath;

    FileSystem::Instance()->MoveFile(inputDir + "landscape.png", scenePath.GetDirectory() + "temp_landscape_lightmap.png", true);

    LightmapsPacker packer;
    packer.SetInputDir(inputDir);

    packer.SetOutputDir(outputDir);
    packer.PackLightmaps(DAVA::GPU_ORIGIN);
    packer.CreateDescriptors();
    packer.ParseSpriteDescriptors();

    BeastProxy::Instance()->UpdateAtlas(beastManager, packer.GetAtlasingData());

    FileSystem::Instance()->MoveFile(scenePath.GetDirectory() + "temp_landscape_lightmap.png", outputDir + "landscape.png", true);
    FileSystem::Instance()->DeleteDirectory(scenePath.GetDirectory() + "$process/");
}

DAVA::FilePath BeastRunner::GetLightmapDirectoryPath()
{
    DAVA::FilePath ret;
    if (NULL != workingScene)
    {
        DAVA::FilePath scenePath = workingScene->GetScenePath();
        ret = scenePath + "_beast/";
    }
    return ret;
}


#endif //#if defined (__DAVAENGINE_BEAST__)
