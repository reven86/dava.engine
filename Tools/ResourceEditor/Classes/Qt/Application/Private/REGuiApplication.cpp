#include "Classes/Qt/Application/REGuiApplication.h"

#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/TextureBrowser/TextureCache.h"

#include "Classes/Deprecated/ControlsFactory.h"

#include "version.h"
#include "QtTools/Utils/AssertGuard.h"
#include "QtTools/Utils/Utils.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "ResourceEditorLauncher.h"

#include "Engine/Engine.h"
#include "Engine/NativeService.h"
#include "Settings/SettingsManager.h"

REGuiApplication::REGuiApplication()
{
    FixOSXFonts();
    MakeAppForeground();
}

void REGuiApplication::OnLoopStarted()
{
    REBaseApplication::OnLoopStarted();

    textureCache = new TextureCache();
    launcher = new ResourceEditorLauncher();

    ToolsAssetGuard::Instance()->Init();
    Themes::InitFromQApplication();

    DAVA::EngineContext* engineContext = engine.GetContext();
    engineContext->localizationSystem->InitWithDirectory("~res:/Strings/");
    engineContext->localizationSystem->SetCurrentLocale("en");
    engineContext->uiControlSystem->SetClearColor(DAVA::Color(.3f, .3f, .3f, 1.f));
    UnpackHelpDoc();

    mainWindow = new QtMainWindow();
    mainWindow->EnableGlobalTimeout(true);
    mainWindow->InjectRenderWidget(engine.GetNativeService()->GetRenderWidget());

    RestoreMenuBar();
    mainWindow->show();
}

void REGuiApplication::OnLoopStopped()
{
    DAVA::SafeDelete(launcher);
    DAVA::SafeDelete(mainWindow);
    ControlsFactory::ReleaseFonts();
    textureCache->Release();

    REBaseApplication::OnLoopStopped();
}

void REGuiApplication::OnWindowCreated(DAVA::Window* w)
{
    DAVA::Renderer::SetDesiredFPS(60);
    DAVA::DynamicBufferAllocator::SetPageSize(16 * 1024 * 1024); // 16 mb

    DVASSERT(mainWindow != nullptr);
    QObject::connect(launcher, &ResourceEditorLauncher::LaunchFinished, [this]()
                     {
                         mainWindow->SetupTitle();
                         mainWindow->OnSceneNew();
                     });
    launcher->Launch();

    REBaseApplication::OnWindowCreated(w);
}

DAVA::eEngineRunMode REGuiApplication::GetEngineMode()
{
    return DAVA::eEngineRunMode::GUI_EMBEDDED;
}

void REGuiApplication::UnpackHelpDoc()
{
    DAVA::EngineContext* engineContext = engine.GetContext();
    DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
    DAVA::FilePath docsPath = DAVA::FilePath(ResourceEditor::DOCUMENTATION_PATH);
    if (editorVer != APPLICATION_BUILD_VERSION || !engineContext->fileSystem->Exists(docsPath))
    {
        DAVA::Logger::FrameworkDebug("Unpacking Help...");
        try
        {
            DAVA::ResourceArchive helpRA("~res:/Help.docs");
            engineContext->fileSystem->DeleteDirectory(docsPath);
            engineContext->fileSystem->CreateDirectory(docsPath, true);
            helpRA.UnpackToFolder(docsPath);
        }
        catch (std::exception& ex)
        {
            DAVA::Logger::Error("can't unpack Help.docs: %s", ex.what());
            DVASSERT(false && "can't upack Help.docs");
        }
    }
    SettingsManager::SetValue(Settings::Internal_EditorVersion, DAVA::VariantType(DAVA::String(APPLICATION_BUILD_VERSION)));
}

void REGuiApplication::FixOSXFonts()
{
    if (QSysInfo::MacintoshVersion != QSysInfo::MV_None && QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
}
