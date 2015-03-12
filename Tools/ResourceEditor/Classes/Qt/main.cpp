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
#include <QFont>
#include <QSysInfo>

#include "version.h"
#include "Main/mainwindow.h"
#include "Main/davaglwidget.h"
#include "Project/ProjectManager.h"
#include "TeamcityOutput/TeamcityOutput.h"
#include "TexturePacker/CommandLineParser.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "TextureCompression/PVRConverter.h"
#include "CommandLine/CommandLineManager.h"
#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "FileSystem/ResourceArchive.h"
#include "TextureBrowser/TextureCache.h"
#include "LicenceDialog/LicenceDialog.h"

#include "Qt/Settings/SettingsManager.h"
#include "Qt/Tools/RunGuard/RunGuard.h"

#include "Deprecated/EditorConfig.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/ControlsFactory.h"

#if defined (__DAVAENGINE_MACOS__)
	#include "MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
	#include "Win32/QtLayerWin32.h"
	#include "Win32/CorePlatformWin32Qt.h"
#endif

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

void UnpackHelpDoc();
void FixOSXFonts();

int main(int argc, char *argv[])
{
	int ret = 0;

#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
	new DAVA::QtLayerMacOS();
	DAVA::PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCLI"));
#elif defined (__DAVAENGINE_WIN32__)
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	DAVA::Core::Run(argc, argv, hInstance);
	new DAVA::QtLayerWin32();
	DAVA::PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCLI.exe"));
#else
	DVASSERT(false && "Wrong platform")
#endif

	DAVA::Logger::Instance()->SetLogFilename("ResEditor.txt");

#ifdef __DAVAENGINE_BEAST__
	new BeastProxyImpl();
#else 
	new BeastProxy();
#endif //__DAVAENGINE_BEAST__


	new SettingsManager();
    //TODO convert old settings to new gpu values
    SettingsManager::UpdateGPUSettings();
    //END of TODO
    
	new EditorConfig();
    ParticleEmitter::FORCE_DEEP_CLONE = true;
    QualitySettingsSystem::Instance()->SetKeepUnusedEntities(true);

	CommandLineManager cmdLine;
	if(cmdLine.IsEnabled())
	{
		Core::Instance()->EnableConsoleMode();
        DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_WARNING);

        new SceneValidator();

#if defined (__DAVAENGINE_MACOS__)
        DAVA::QtLayerMacOS *qtLayer = (DAVA::QtLayerMacOS *) DAVA::QtLayer::Instance();
        qtLayer->InitializeGlWindow(nullptr, 0, 0);

        DAVA::QtLayer::Instance()->Resize(0, 0);
#elif defined (__DAVAENGINE_WIN32__)
        QApplication a(argc, argv);
        
        DavaGLWidget* davaGL = new DavaGLWidget();
#else
        DVASSERT(false && "Wrong platform");
#endif //#if defined (__DAVAENGINE_MACOS__)
        
        RenderManager::Instance()->Init(0, 0);

		cmdLine.InitalizeTool();
		if(!cmdLine.IsToolInitialized())
		{
			cmdLine.PrintUsageForActiveTool();
		}
		else
		{
            //Trick for correct loading of sprites.
            VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
            VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1, 1, "Gfx");
            
			cmdLine.Process();
			cmdLine.PrintResults();
		}

#if defined (__DAVAENGINE_MACOS__)
#elif defined (__DAVAENGINE_WIN32__)
        SafeDelete(davaGL);
#endif //defined (__DAVAENGINE_WIN32__)
        
		SceneValidator::Instance()->Release();
	}
    else
    {
        
#ifdef Q_OS_MAC
        FixOSXFonts();  // Must be called before creating QApplication instance
#endif
        
        QApplication a(argc, argv);

        a.setAttribute(Qt::AA_UseHighDpiPixmaps);

        const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
        const QString appUidPath = QCryptographicHash::hash( (appUid + a.applicationDirPath() ).toUtf8(), QCryptographicHash::Sha1 ).toHex();
        RunGuard runGuard( appUidPath );

        if ( runGuard.tryToRun() )
        {
            LicenceDialog licenceDlg;
            if ( licenceDlg.process() )
            {
                new SceneValidator();
                new TextureCache();
                
                LocalizationSystem::Instance()->SetCurrentLocale("en");
                LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");
                
                DAVA::Texture::SetDefaultGPU((eGPUFamily) SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsInt32());
                
                // check and unpack help documents
                UnpackHelpDoc();
                
                // create and init UI
                new QtMainWindow();
                QtMainWindow::Instance()->EnableGlobalTimeout(true);
                QtMainWindow::Instance()->show();
                ProjectManager::Instance()->ProjectOpenLast();
                if(ProjectManager::Instance()->IsOpened())
                    QtMainWindow::Instance()->OnSceneNew();
                
                DAVA::Logger::Instance()->Log(DAVA::Logger::LEVEL_INFO, QString( "Qt version: %1" ).arg( QT_VERSION_STR ).toStdString().c_str() );
                
                // start app
                ret = a.exec();
                
                QtMainWindow::Instance()->Release();
                ControlsFactory::ReleaseFonts();
                
                SceneValidator::Instance()->Release();
                TextureCache::Instance()->Release();
            }
        }
    }

	EditorConfig::Instance()->Release();
	SettingsManager::Instance()->Release();
	BeastProxy::Instance()->Release();
	DAVA::QtLayer::Instance()->Release();
	DAVA::Core::Instance()->Release();

    return ret;
}

void UnpackHelpDoc()
{
	DAVA::String editorVer =SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
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
	SettingsManager::SetValue(Settings::Internal_EditorVersion, VariantType(String(RESOURCE_EDITOR_VERSION)));
}

void FixOSXFonts()
{
#ifdef Q_OS_MAC
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        QFont::insertSubstitution( ".Lucida Grande UI", "Lucida Grande" );
    }
#endif
}