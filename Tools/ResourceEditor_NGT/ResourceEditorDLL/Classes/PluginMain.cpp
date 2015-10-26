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

#include "SceneEnumerator.h"
#include "SceneObserver.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/Project/ProjectManager.h"
#include "Classes/Qt/TextureBrowser/TextureCache.h"
#include "Classes/Qt/LicenceDialog/LicenceDialog.h"
#include "Classes/Qt/Settings/SettingsManager.h"
#include "Classes/CommandLine/CommandLineManager.h"
#include "Classes/CommandLine/TextureDescriptor/TextureDescriptorUtils.h"
#include "Classes/Deprecated/EditorConfig.h"
#include "Classes/Deprecated/SceneValidator.h"
#include "Classes/Deprecated/ControlsFactory.h"

#include "Tools/version.h"
#include "Tools/QtTools/DavaGLWidget/davaglwidget.h"
#include "Tools/QtTools/RunGuard/RunGuard.h"
#include "Tools/QtTools/FrameworkBinding/DavaLoop.h"
#include "Tools/QtTools/FrameworkBinding/FrameworkLoop.h"
#include "Tools/CommandLine/CommandLineParser.h"
#include "Tools/TeamcityOutput/TeamcityOutput.h"
#include "Tools/TexturePacker/ResourcePacker2D.h"
#include "Tools/TextureCompression/PVRConverter.h"

#include "FileSystem/ResourceArchive.h"
#include "Platform/Qt5/QtLayer.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "Classes/Beast/BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include <QCryptographicHash>
#include <QFont>

#include <core_generic_plugin/generic_plugin.hpp>
#include <core_generic_plugin/interfaces/i_command_line_parser.hpp>
#include <core_qt_common/qt_window.hpp>
#include <core_qt_common/i_qt_framework.hpp>
#include <core_ui_framework/i_ui_application.hpp>

namespace
{
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

void UnpackHelpDoc()
{
    DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
    DAVA::FilePath docsPath = FilePath(ResourceEditor::DOCUMENTATION_PATH);
    if (editorVer != APPLICATION_BUILD_VERSION || !docsPath.Exists())
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
    SettingsManager::SetValue(Settings::Internal_EditorVersion, VariantType(String(APPLICATION_BUILD_VERSION)));
}
}

class ResourceEditorPlugin : public PluginMain
{
    std::vector<IInterface*> typesId;

public:
    ResourceEditorPlugin(IComponentContext& contextManager)
    {
    }

    bool PostLoad(IComponentContext& context) override
    {
        typesId.emplace_back(context.registerInterface<INTERFACE_VERSION(ISceneObserver, 0, 0)>(new SceneObserver()));
        typesId.emplace_back(context.registerInterface<INTERFACE_VERSION(ISceneEnumerator, 0, 0)>(new SceneEnumerator()));
        return true;
    }

    void Initialise(IComponentContext& context) override
    {
        ICommandLineParser* parser = context.queryInterface<ICommandLineParser>();
        if (parser == nullptr)
        {
            DAVA::Logger::Error("Can't query ICommandLineParser");
            return;
        }

        InitializeRE(*parser);
        ShowMainWindow();
    }

    bool Finalise(IComponentContext& context) override
    {
        FinaliseRE();
        return true;
    }

    void Unload(IComponentContext& context) override
    {
        std::for_each(typesId.begin(), typesId.end(), [&context](IInterface* typeId) { context.deregisterInterface(typeId); });
    }

private:
    void InitializeRE(ICommandLineParser& cmdParser)
    {
#if defined(__DAVAENGINE_MACOS__)
        const String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
        const String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif

        DAVA::Core::Run(cmdParser.argc(), cmdParser.argv());
        new DAVA::QtLayer();
        DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);

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
    }

    void ShowMainWindow()
    {
#ifdef Q_OS_MAC
        // Must be called before creating QApplication instance
        FixOSXFonts();
        DAVA::QtLayer::MakeAppForeground(false);
#endif

        const QString appUid = "{AA5497E4-6CE2-459A-B26F-79AAF05E0C6B}";
        const QString appUidPath = QCryptographicHash::hash((appUid + QApplication::applicationDirPath()).toUtf8(), QCryptographicHash::Sha1).toHex();
        RunGuard runGuard(appUidPath);
        if (!runGuard.tryToRun())
            return;

        //a.setAttribute(Qt::AA_UseHighDpiPixmaps);
        //a.setAttribute(Qt::AA_ShareOpenGLContexts);

        Q_INIT_RESOURCE(QtToolsResources);
        Q_INIT_RESOURCE(QtIcons);

        new SceneValidator();
        new TextureCache();

        LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings/");
        LocalizationSystem::Instance()->SetCurrentLocale("en");

        int32 val = SettingsManager::GetValue(Settings::Internal_TextureViewGPU).AsUInt32();
        eGPUFamily family = static_cast<eGPUFamily>(val);
        DAVA::Texture::SetDefaultGPU(family);

        // check and unpack help documents
        UnpackHelpDoc();

#ifdef Q_OS_MAC
        QTimer::singleShot(0, [] { DAVA::QtLayer::MakeAppForeground(); });
        QTimer::singleShot(0, [] { DAVA::QtLayer::RestoreMenuBar(); });
#endif

        new DavaLoop();
        new FrameworkLoop();

        QTimer::singleShot(0, [&] {
            // create and init UI
            QtMainWindow* mainWindow = new QtMainWindow();

            mainWindow->EnableGlobalTimeout(true);
            glWidget = QtMainWindow::Instance()->GetSceneWidget()->GetDavaWidget();
            FrameworkLoop::Instance()->SetOpenGLWindow(glWidget);

            ProjectManager::Instance()->ProjectOpenLast();
            QObject::connect(glWidget, &DavaGLWidget::Initialized, ProjectManager::Instance(), &ProjectManager::UpdateParticleSprites);
            QObject::connect(glWidget, &DavaGLWidget::Initialized, ProjectManager::Instance(), &ProjectManager::OnSceneViewInitialized);
            QObject::connect(glWidget, &DavaGLWidget::Initialized, mainWindow, &QtMainWindow::OnSceneNew, Qt::QueuedConnection);

            IQtFramework* qtFramework = Context::queryInterface<IQtFramework>();
            DVASSERT(qtFramework != nullptr);

            mainWindow_ = new QtWindow(*qtFramework, std::unique_ptr<QMainWindow>(mainWindow));
            mainWindow_->show();

            IUIApplication* application = Context::queryInterface<IUIApplication>();
            DVASSERT(application != nullptr);

            application->addWindow(*mainWindow_);

            DAVA::Logger::Instance()->Log(DAVA::Logger::LEVEL_INFO, QString("Qt version: %1").arg(QT_VERSION_STR).toStdString().c_str());

            DavaLoop::Instance()->StartLoop(FrameworkLoop::Instance());
        });
    }

    void FinaliseRE()
    {
        glWidget->setParent(nullptr);
        static_cast<QtMainWindow*>(mainWindow_->window())->Release();

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

    DavaGLWidget* glWidget = nullptr;
    QtWindow* mainWindow_ = nullptr;
};

PLG_CALLBACK_FUNC(ResourceEditorPlugin)