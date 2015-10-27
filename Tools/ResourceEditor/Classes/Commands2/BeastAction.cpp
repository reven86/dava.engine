/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "BeastAction.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Main/mainwindow.h"
#include "Beast/BeastProxy.h"
#include "Beast/LightmapsPacker.h"
#include "Qt/Settings/SettingsManager.h"
#include "CommandLine/SceneUtils/SceneUtils.h"

#include "DAVAEngine.h"

using namespace DAVA;

#if defined (__DAVAENGINE_BEAST__)

//Beast
BeastAction::BeastAction(SceneEditor2 *scene, const DAVA::FilePath& _outputPath, BeastProxy::eBeastMode mode, QtWaitDialog *_waitDialog)
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
	bool canceled = false;

	if(NULL != waitDialog)
	{
		waitDialog->Show("Beast process", "Starting Beast", true, true);
		waitDialog->SetRange(0, 100);
		waitDialog->EnableCancel(false);
	}

	Start();

	while( Process() == false )
	{
		if(canceled) 
		{
			BeastProxy::Instance()->Cancel(beastManager);
			break;
		}
		else
		{
			if(NULL != waitDialog)
			{
				waitDialog->SetValue(BeastProxy::Instance()->GetCurTaskProcess(beastManager));
				waitDialog->SetMessage(BeastProxy::Instance()->GetCurTaskName(beastManager).c_str());
				waitDialog->EnableCancel(true);

				canceled = waitDialog->WasCanceled();
			}
		}

        if (Core::Instance()->IsConsoleMode())
        {
            RenderObjectsFlusher::Flush();
        }
        Sleep(15);
    }

	if(NULL != waitDialog)
	{
		waitDialog->EnableCancel(false);
	}

    Finish(canceled);

    if (waitDialog != nullptr)
    {
		waitDialog->Reset();
	}
}

void BeastAction::Start()
{
	startTime = SystemTimer::Instance()->AbsoluteMS();

	FilePath path = GetLightmapDirectoryPath();
    if(beastMode == BeastProxy::MODE_LIGHTMAPS)
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

	Landscape *land = FindLandscape(workingScene);
	if(land)
	{
        FilePath textureName = land->GetMaterial()->GetEffectiveTexture(DAVA::Landscape::TEXTURE_COLOR)->GetPathname();
        if (textureName.Exists())
        {
            textureName.ReplaceFilename("temp_beast.png");
            FileSystem::Instance()->DeleteFile(textureName);
        }
	}

    FileSystem::Instance()->DeleteDirectory(FileSystem::Instance()->GetCurrentWorkingDirectory() + "temp_beast/");
    if(beastMode == BeastProxy::MODE_LIGHTMAPS)
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
	if(NULL != workingScene)
	{
		DAVA::FilePath scenePath = workingScene->GetScenePath();
		ret = scenePath + "_beast/";
	}
	return ret;
}


#endif //#if defined (__DAVAENGINE_BEAST__)


