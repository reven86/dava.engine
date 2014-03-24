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



#include "DAVAEngine.h"
#include <QApplication>
#include <QCryptographicHash>

#include "version.h"
#include "Main/mainwindow.h"
#include "Main/davaglwidget.h"
#include "Project/ProjectManager.h"
#include "Utils/TeamcityOutput.h"
#include "TexturePacker/CommandLineParser.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "TextureCompression/PVRConverter.h"
#include "CommandLine/CommandLineManager.h"
#include "CommandLine/SceneExporter/SceneExporter.h"
#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "FileSystem/ResourceArchive.h"
#include "TextureBrowser/TextureCache.h"

#include "Qt/Settings/SettingsManager.h"
#include "Qt/Tools/RunGuard/RunGuard.h"

#include "Deprecated/EditorConfig.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/ControlsFactory.h"

#include "Scene/FogSettingsChangedReceiver.h"

#if defined (__DAVAENGINE_MACOS__)
	#include "Platform/Qt/MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
	#include "Platform/Qt/Win32/QtLayerWin32.h"
	#include "Platform/Qt/Win32/CorePlatformWin32.h"
#endif

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

void UnpackHelpDoc();

int main(int argc, char *argv[])
{
	int ret = 0;

    QApplication a(argc, argv);

#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
	new DAVA::QtLayerMacOS();
	DAVA::PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCL"));
#elif defined (__DAVAENGINE_WIN32__)
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	DAVA::Core::Run(argc, argv, hInstance);
	new DAVA::QtLayerWin32();
	DAVA::PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCL.exe"));
#else
	DVASSERT(false && "Wrong platform")
#endif

// GUI instance is already started
        

#ifdef __DAVAENGINE_BEAST__
	new BeastProxyImpl();
#else 
	new BeastProxy();
#endif //__DAVAENGINE_BEAST__

    const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
    const QString appUidPath = QCryptographicHash::hash( (appUid + a.applicationDirPath() ).toUtf8(), QCryptographicHash::Sha1 ).toHex();
    RunGuard runGuard( appUidPath );
	CommandLineManager cmdLine;

	if(cmdLine.IsEnabled())
	{
        DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_WARNING);
        
		new SceneValidator();
		DavaGLWidget* davaGL = new DavaGLWidget();

		//DAVA::TeamcityOutput *out = new DAVA::TeamcityOutput();
		//DAVA::Logger::AddCustomOutput(out);

		cmdLine.InitalizeTool();
		if(!cmdLine.IsToolInitialized())
		{
			cmdLine.PrintUsageForActiveTool();
		}
		else
		{
            //Trick for correct loading of sprites.
            Core::Instance()->UnregisterAllAvailableResourceSizes();
            Core::Instance()->RegisterAvailableResourceSize(1, 1, "Gfx");
            
			cmdLine.Process();
			cmdLine.PrintResults();
		}

		SafeDelete(davaGL);
		SceneValidator::Instance()->Release();
	}
	else if ( runGuard.tryToRun() )
	{
		new SettingsManager();
		new EditorConfig();
		new SceneValidator();
        new TextureCache();
		new FogSettingsChangedReceiver();

		LocalizationSystem::Instance()->SetCurrentLocale("en");
		LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");

		DAVA::Logger::Instance()->SetLogFilename("ResEditor.txt");

		DAVA::Texture::SetDefaultGPU((eGPUFamily)SettingsManager::Instance()->GetValue("TextureViewGPU", SettingsManager::INTERNAL).AsInt32());

		// check and unpack help documents
		UnpackHelpDoc();

		// create and init UI
		new QtMainWindow();
		QtMainWindow::Instance()->EnableGlobalTimeout(true);
		QtMainWindow::Instance()->show();
		ProjectManager::Instance()->ProjectOpenLast();
        if(ProjectManager::Instance()->IsOpened())
            QtMainWindow::Instance()->OnSceneNew();

		// start app
		ret = a.exec();

		QtMainWindow::Instance()->Release();
		ControlsFactory::ReleaseFonts();

		SceneValidator::Instance()->Release();
		EditorConfig::Instance()->Release();
		SettingsManager::Instance()->Release();
        TextureCache::Instance()->Release();
		FogSettingsChangedReceiver::Instance()->Release();
	}

	BeastProxy::Instance()->Release();
	DAVA::QtLayer::Instance()->Release();
	DAVA::Core::Instance()->ReleaseSingletons();
	DAVA::Core::Instance()->Release();

	return ret;
}

void UnpackHelpDoc()
{
	DAVA::String editorVer =SettingsManager::Instance()->GetValue("editor.version", SettingsManager::INTERNAL).AsString();
	DAVA::FilePath docsPath = FilePath(ResourceEditor::DOCUMENTATION_PATH);
	if(editorVer != RESOURCE_EDITOR_VERSION || !docsPath.Exists())
	{
		DAVA::Logger::Info("Unpacking Help...");
		DAVA::ResourceArchive * helpRA = new DAVA::ResourceArchive();
		if(helpRA->Open("~res:/Help.docs"))
		{
			DAVA::FileSystem::Instance()->DeleteDirectory(docsPath);
			DAVA::FileSystem::Instance()->CreateDirectory(docsPath, true);
			helpRA->UnpackToFolder(docsPath);
		}
		DAVA::SafeRelease(helpRA);
	}
	SettingsManager::Instance()->SetValue("editor.version", VariantType(String(RESOURCE_EDITOR_VERSION)), SettingsManager::INTERNAL);
}
