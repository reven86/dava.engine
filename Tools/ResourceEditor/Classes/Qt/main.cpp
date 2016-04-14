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
#include "QtTools/DavaGLWidget/davaglwidget.h"
#include "Project/ProjectManager.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "TextureCompression/PVRConverter.h"
#include "CommandLine/CommandLineManager.h"
#include "FileSystem/ResourceArchive.h"
#include "TextureBrowser/TextureCache.h"

#include "Qt/Settings/SettingsManager.h"
#include "QtTools/RunGuard/RunGuard.h"

#include "Deprecated/EditorConfig.h"
#include "Deprecated/SceneValidator.h"
#include "Deprecated/ControlsFactory.h"

#include "Platform/Qt5/QtLayer.h"

#include "ResourceEditorLauncher.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

void UnpackHelpDoc();
void FixOSXFonts();

void RunConsole(int argc, char* argv[], CommandLineManager& cmdLine);
void RunGui(int argc, char* argv[], CommandLineManager& cmdLine);

int main(int argc, char* argv[])
{
#if defined(__DAVAENGINE_MACOS__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif

    DAVA::Core::Run(argc, argv);
    DAVA::QtLayer qtLayer;
    DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);

    DAVA::Logger::Instance()->SetLogFilename("ResEditor.txt");

#ifdef __DAVAENGINE_BEAST__
    BeastProxyImpl beastProxyImpl;
#else
    BeastProxy beastProxy;
#endif //__DAVAENGINE_BEAST__

    DAVA::ParticleEmitter::FORCE_DEEP_CLONE = true;
    DAVA::QualitySettingsSystem::Instance()->SetKeepUnusedEntities(true);

    int exitCode = 0;
    {
        EditorConfig config;
        SettingsManager settingsManager;
        SettingsManager::UpdateGPUSettings();
        SceneValidator sceneValidator;

        CommandLineManager cmdLine(argc, argv);
        if (cmdLine.IsEnabled())
        {
            RunConsole(argc, argv, cmdLine);
        }
        else if (argc == 1)
        {
            RunGui(argc, argv, cmdLine);
        }
        else
        {
            exitCode = 1; //wrong commandLine
        }
    }

    return exitCode;
}

void RunConsole(int argc, char* argv[], CommandLineManager& cmdLineManager)
{
#if defined(__DAVAENGINE_MACOS__)
    DAVA::QtLayer::MakeAppForeground(false);
#elif defined(__DAVAENGINE_WIN32__)
//    WinConsoleIOLocker locker; //temporary disabled because of freezes of Windows Console
#endif //platforms

    DAVA::Core::Instance()->EnableConsoleMode();
    DAVA::Logger::Instance()->EnableConsoleMode();
    DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_INFO);

    QApplication a(argc, argv);

    DavaGLWidget glWidget;
    glWidget.MakeInvisible();

    DAVA::Logger::Info(QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());

    // Delayed initialization throught event loop
    glWidget.show();
#ifdef Q_OS_WIN
    QObject::connect(&glWidget, &DavaGLWidget::Initialized, &a, &QApplication::quit);
    a.exec();
#endif
    glWidget.hide();

    //Trick for correct loading of sprites.
    DAVA::VirtualCoordinatesSystem::Instance()->UnregisterAllAvailableResourceSizes();
    DAVA::VirtualCoordinatesSystem::Instance()->RegisterAvailableResourceSize(1, 1, "Gfx");

    cmdLineManager.Process();
}

void RunGui(int argc, char* argv[], CommandLineManager& cmdLine)
{
#ifdef Q_OS_MAC
    // Must be called before creating QApplication instance
    FixOSXFonts();
    DAVA::QtLayer::MakeAppForeground(false);
#endif

    QApplication a(argc, argv);
    a.setOrganizationName("DAVA");
    a.setApplicationName("Resource Editor");
    //scope with editor variables
    {
        const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
        const QString appUidPath = QCryptographicHash::hash((appUid + QApplication::applicationDirPath()).toUtf8(), QCryptographicHash::Sha1).toHex();
        RunGuard runGuard(appUidPath);
        if (!runGuard.tryToRun())
            return;

        a.setAttribute(Qt::AA_UseHighDpiPixmaps);
        a.setAttribute(Qt::AA_ShareOpenGLContexts);

        Q_INIT_RESOURCE(QtToolsResources);

        TextureCache textureCache;

        DAVA::LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");
        DAVA::LocalizationSystem::Instance()->SetCurrentLocale("en");

        // check and unpack help documents
        UnpackHelpDoc();

    #ifdef Q_OS_MAC
        QTimer::singleShot(0, [] { DAVA::QtLayer::MakeAppForeground(); });
        QTimer::singleShot(0, [] { DAVA::QtLayer::RestoreMenuBar(); });
    #endif

        ResourceEditorLauncher launcher;
        {
            QtMainWindow mainWindow;

            QTimer::singleShot(0, [&]
                               {
                                   // create and init UI
                                   mainWindow.EnableGlobalTimeout(true);
                                   auto glWidget = QtMainWindow::Instance()->GetSceneWidget()->GetDavaWidget();

                                   QObject::connect(glWidget, &DavaGLWidget::Initialized, &launcher, &ResourceEditorLauncher::Launch);

                                   mainWindow.show();

                                   DAVA::Logger::Instance()->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());
                               });
            // start app
            a.exec();
        }
    }
    ControlsFactory::ReleaseFonts();
}

void UnpackHelpDoc()
{
    DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
    DAVA::FilePath docsPath = DAVA::FilePath(ResourceEditor::DOCUMENTATION_PATH);
    if (editorVer != APPLICATION_BUILD_VERSION || !DAVA::FileSystem::Instance()->Exists(docsPath))
    {
        DAVA::Logger::FrameworkDebug("Unpacking Help...");
        DAVA::ResourceArchive* helpRA = new DAVA::ResourceArchive();
        if (helpRA->Open("~res:/Help.docs"))
        {
            DAVA::FileSystem::Instance()->DeleteDirectory(docsPath);
            DAVA::FileSystem::Instance()->CreateDirectory(docsPath, true);
            helpRA->UnpackToFolder(docsPath);
        }
        DAVA::SafeRelease(helpRA);
    }
    SettingsManager::SetValue(Settings::Internal_EditorVersion, DAVA::VariantType(DAVA::String(APPLICATION_BUILD_VERSION)));
}

void FixOSXFonts()
{
#ifdef Q_OS_MAC
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif
}
