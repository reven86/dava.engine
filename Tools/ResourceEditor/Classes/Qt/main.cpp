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
#include "QtTools/DavaGLWidget/davaglwidget.h"
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

#include "Platform/Qt5/QtLayer.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include "QtTools/FrameworkBinding/DavaLoop.h"
#include "QtTools/FrameworkBinding/FrameworkLoop.h"

#include <QDebug>

void UnpackHelpDoc();
void FixOSXFonts();

void RunConsole( int argc, char *argv[], CommandLineManager& cmdLine );
void RunGui( int argc, char *argv[], CommandLineManager& cmdLine );

int main(int argc, char *argv[])
{
#if defined (__DAVAENGINE_MACOS__)
    const String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined (__DAVAENGINE_WIN32__)
    const String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif

    DAVA::Core::Run( argc, argv );
    new DAVA::QtLayer();
    DAVA::PVRConverter::Instance()->SetPVRTexTool( pvrTexToolPath );

	DAVA::Logger::Instance()->SetLogFilename("ResEditor.txt");

#ifdef __DAVAENGINE_BEAST__
	new BeastProxyImpl();
#else 
	new BeastProxy();
#endif //__DAVAENGINE_BEAST__

	new SettingsManager();
    SettingsManager::UpdateGPUSettings();
    
	new EditorConfig();
    ParticleEmitter::FORCE_DEEP_CLONE = true;
    QualitySettingsSystem::Instance()->SetKeepUnusedEntities(true);

	CommandLineManager cmdLine;

	if(cmdLine.IsEnabled())
	{
        RunConsole( argc, argv, cmdLine );
	}
    else
    {
        RunGui( argc, argv, cmdLine );
    }

    return 0;
}

/*
 * Temporary (hopefully!) solution for deferred deletion in RHI.
 * Certain command line tools will time to time call 
 * this function in order to release resources.
 */
void EngineHelperCallback()
{
	static const rhi::HTexture nullTexture;
	static const rhi::Viewport nullViewport(0, 0, 1, 1);
	RenderHelper::CreateClearPass(nullTexture, 0, DAVA::Color::Clear, nullViewport);
	rhi::Present();
}

void RunConsole( int argc, char *argv[], CommandLineManager& cmdLine )
{
#ifdef Q_OS_MAC
    DAVA::QtLayer::MakeAppForeground(false);
#endif
    
    Core::Instance()->EnableConsoleMode();
    DAVA::Logger::Instance()->SetLogLevel( DAVA::Logger::LEVEL_WARNING );

    QApplication a( argc, argv );

    new SceneValidator();

    new DavaLoop();
    new FrameworkLoop();

    auto glWidget = new DavaGLWidget();
    glWidget->MakeInvisible();

    FrameworkLoop::Instance()->SetOpenGLWindow( glWidget );

    DAVA::Logger::Instance()->Log( DAVA::Logger::LEVEL_INFO, QString( "Qt version: %1" ).arg( QT_VERSION_STR ).toStdString().c_str() );

    // Delayed initialization throught event loop
    glWidget->show();
#ifdef Q_OS_WIN
    FrameworkLoop::Instance()->Context();   // Force context initialization
    QObject::connect( glWidget, &DavaGLWidget::Initialized, &a, &QApplication::quit );
    QTimer::singleShot( 0, glWidget, &DavaGLWidget::OnWindowExposed );
    a.exec();
#endif
    glWidget->hide();

    cmdLine.InitalizeTool();
    if ( !cmdLine.IsToolInitialized() )
    {
		cmdLine.PrintResults();
        cmdLine.PrintUsageForActiveTool();
    }
    else
    {
        //Trick for correct loading of sprites.
        VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
        VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize( 1, 1, "Gfx" );

		cmdLine.Process(&EngineHelperCallback);
        cmdLine.PrintResults();
    }

    SceneValidator::Instance()->Release();
    EditorConfig::Instance()->Release();
    SettingsManager::Instance()->Release();
    BeastProxy::Instance()->Release();
    Core::Instance()->Release();

    FrameworkLoop::Instance()->Release();
    QtLayer::Instance()->Release();
    DavaLoop::Instance()->Release();

    delete glWidget;
}

void RunGui( int argc, char *argv[], CommandLineManager& cmdLine )
{
#ifdef Q_OS_MAC
    // Must be called before creating QApplication instance
    FixOSXFonts();
    DAVA::QtLayer::MakeAppForeground(false);
#endif
 
    QApplication a( argc, argv );

    const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
    const QString appUidPath = QCryptographicHash::hash( ( appUid + QApplication::applicationDirPath() ).toUtf8(), QCryptographicHash::Sha1 ).toHex();
    RunGuard runGuard( appUidPath );
    if ( !runGuard.tryToRun() )
        return;

    a.setAttribute( Qt::AA_UseHighDpiPixmaps );
    a.setAttribute( Qt::AA_ShareOpenGLContexts );

    Q_INIT_RESOURCE( QtToolsResources );

    new SceneValidator();
    new TextureCache();

    LocalizationSystem::Instance()->InitWithDirectory( "~res:/Strings/" );
    LocalizationSystem::Instance()->SetCurrentLocale( "en" );

    DAVA::Texture::SetDefaultGPU( static_cast<eGPUFamily>(SettingsManager::GetValue( Settings::Internal_TextureViewGPU ).AsInt32()) );

    // check and unpack help documents
    UnpackHelpDoc();

#ifdef Q_OS_MAC
    QTimer::singleShot(0, []{ DAVA::QtLayer::MakeAppForeground();    } );
    QTimer::singleShot(0, []{ DAVA::QtLayer::RestoreMenuBar();       } );
#endif
    
    new DavaLoop();
    new FrameworkLoop();
    
    DavaGLWidget *glWidget = nullptr;
    QtMainWindow *mainWindow = nullptr;

    QTimer::singleShot(0, [&]
    {
        // create and init UI
        mainWindow = new QtMainWindow();
        
        mainWindow->EnableGlobalTimeout( true );
        glWidget = QtMainWindow::Instance()->GetSceneWidget()->GetDavaWidget();
        FrameworkLoop::Instance()->SetOpenGLWindow( glWidget );
        
        ProjectManager::Instance()->ProjectOpenLast();
        QObject::connect( glWidget, &DavaGLWidget::Initialized, ProjectManager::Instance(), &ProjectManager::UpdateParticleSprites );
        QObject::connect( glWidget, &DavaGLWidget::Initialized, ProjectManager::Instance(), &ProjectManager::OnSceneViewInitialized );
        QObject::connect( glWidget, &DavaGLWidget::Initialized, mainWindow, &QtMainWindow::OnSceneNew, Qt::QueuedConnection );
        
        mainWindow->show();
        
        DAVA::Logger::Instance()->Log( DAVA::Logger::LEVEL_INFO, QString( "Qt version: %1" ).arg( QT_VERSION_STR ).toStdString().c_str() );
        
        DavaLoop::Instance()->StartLoop( FrameworkLoop::Instance() );
    } );
    
    
    // start app
    QApplication::exec();
 
    glWidget->setParent( nullptr );
    mainWindow->Release();

    TextureCache::Instance()->Release();
    SceneValidator::Instance()->Release();
    EditorConfig::Instance()->Release();
    SettingsManager::Instance()->Release();
    BeastProxy::Instance()->Release();
    Core::Instance()->Release();

    ControlsFactory::ReleaseFonts();

    FrameworkLoop::Instance()->Release();
    QtLayer::Instance()->Release();
    DavaLoop::Instance()->Release();
    delete glWidget;
}

void UnpackHelpDoc()
{
    DAVA::String editorVer = SettingsManager::GetValue( Settings::Internal_EditorVersion ).AsString();
    DAVA::FilePath docsPath = FilePath( ResourceEditor::DOCUMENTATION_PATH );
    if ( editorVer != APPLICATION_VERSION || !docsPath.Exists() )
    {
        DAVA::Logger::FrameworkDebug( "Unpacking Help..." );
        DAVA::ResourceArchive * helpRA = new DAVA::ResourceArchive();
        if ( helpRA->Open( "~res:/Help.docs" ) )
        {
            DAVA::FileSystem::Instance()->DeleteDirectory( docsPath );
            DAVA::FileSystem::Instance()->CreateDirectory( docsPath, true );
            helpRA->UnpackToFolder( docsPath );
        }
        DAVA::SafeRelease( helpRA );
    }
    SettingsManager::SetValue( Settings::Internal_EditorVersion, VariantType( String( APPLICATION_VERSION ) ) );
}

void FixOSXFonts()
{
#ifdef Q_OS_MAC
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        QFont::insertSubstitution( ".Lucida Grande UI", "Lucida Grande" );
    }
#endif
}
