#include "BeastAction.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Main/mainwindow.h"
#include "Beast/BeastProxy.h"
#include "Beast/LightmapsPacker.h"
#include "Qt/Settings/SettingsManager.h"
#include "CommandLine/SceneUtils/SceneUtils.h"

#include "DAVAEngine.h"

#if defined(__DAVAENGINE_BEAST__)

#include "SceneParser.h"

using namespace DAVA;

#include "SceneParser.h"

//Beast
BeastAction::BeastAction(SceneEditor2* scene, const DAVA::FilePath& _outputPath, BeastProxy::eBeastMode mode, QtWaitDialog* _waitDialog)
    : CommandAction(CMDID_BEAST, "Beast")
    , workingScene(scene)
    , waitDialog(_waitDialog)
    , outputPath(_outputPath)
    , beastMode(mode)
{
    outputPath.MakeDirectoryPathname();
    beastManager = BeastProxy::Instance()->CreateManager();
    BeastProxy::Instance()->SetMode(beastManager, mode);
}

BeastAction::~BeastAction()
{
    BeastProxy::Instance()->SafeDeleteManager(&beastManager);
}

void BeastAction::Redo()
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

        if (Core::Instance()->IsConsoleMode())
        {
            RenderObjectsFlusher::Flush();
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

void BeastAction::Start()
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

bool BeastAction::Process()
{
    BeastProxy::Instance()->Update(beastManager);
    return BeastProxy::Instance()->IsJobDone(beastManager);
}

void BeastAction::Finish(bool canceled)
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

void BeastAction::PackLightmaps()
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

DAVA::FilePath BeastAction::GetLightmapDirectoryPath()
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
