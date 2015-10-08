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


#include "GameCore.h"

#include "Grid/GridVisualizer.h"
#include "Ruler/RulerController.h"

//#include "ScreenManager.h"
#include "EditorSettings.h"
#include "Helpers/ResourcesManageHelper.h"
#include "FileSystem/ResourceArchive.h"
#include "Autotesting/AutotestingSystem.h"
#include "Version.h"

using namespace DAVA;

GameCore::GameCore()
    : cursor(nullptr)
{
    // Editor Settings might be used by any singleton below during initialization, so
    // initialize it before any other one.
    new EditorSettings();

    new GridVisualizer();
    new RulerController();
    new AutotestingSystem();

	// Unpack the help data, if needed.
	UnpackHelp();

	//Initialize internal resources of application
	ResourcesManageHelper::InitInternalResources();
}

GameCore::~GameCore()
{
    RulerController::Instance()->Release();
    GridVisualizer::Instance()->Release();

    EditorSettings::Instance()->Release();
        
    AutotestingSystem::Instance()->Release();
}

void GameCore::OnAppStarted()
{
    cursor = nullptr;
	RenderManager::Instance()->SetFPS(60);
}

void GameCore::OnAppFinished()
{
	SafeRelease(cursor);
}

void GameCore::OnSuspend()
{
    ApplicationCore::OnSuspend();
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{
	
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
    RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
}

void GameCore::Update(float32 timeElapsed)
{	
//	if (!cursor)
//	{
//		cursor = Cursor::Create("~res:/Cursor/cursor1.png", Vector2(6, 0));
//		RenderManager::Instance()->SetCursor(cursor);
//	}
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}

void GameCore::UnpackHelp()
{
	//Unpack Help to Documents.
    String editorVer = EditorSettings::Instance()->GetUIEditorVersion();
	FilePath docsPath = FilePath(ResourcesManageHelper::GetDocumentationPath().toStdString());
    if (editorVer != APPLICATION_BUILD_VERSION || !docsPath.Exists())
    {
        ResourceArchive * helpRA = new ResourceArchive();
		if(helpRA->Open("~res:/Help.docs"))
		{
			FileSystem::Instance()->DeleteDirectory(docsPath);
			FileSystem::Instance()->CreateDirectory(docsPath, true);
		
			helpRA->UnpackToFolder(docsPath);
            EditorSettings::Instance()->SetUIEditorVersion(APPLICATION_BUILD_VERSION);
        }

        SafeRelease(helpRA);
	}
}
