#include "Classes/Qt/Application/REModule.h"

#include "ResourceEditorLauncher.h"
#include "Main/mainwindow.h"
#include "TextureCache.h"
#include "QtTools/Utils/Themes/Themes.h"

#include "WindowSubSystem/Private/UIManager.h"
#include "DataProcessing/DataNode.h"
#include "version.h"
#include "Deprecated/ControlsFactory.h"

#include <QPointer>

namespace REModuleDetail
{
class REGlobalData : public DAVA::TArc::DataNode
{
public:
    REGlobalData()
        : windowKey(DAVA::FastName("ResourceEditor"))
    {
        textureCache = new TextureCache();
        mainWindow = new QtMainWindow();
        launcher = new ResourceEditorLauncher();
    }

    ~REGlobalData()
    {
        DAVA::SafeDelete(launcher);
        DAVA::SafeRelease(textureCache);

        if (!mainWindow.isNull())
        {
            QtMainWindow* mainWindowPointer = mainWindow.data();
            mainWindow.clear();
            DAVA::SafeDelete(mainWindowPointer);
        }
    }

    ResourceEditorLauncher* launcher = nullptr;
    TextureCache* textureCache = nullptr;
    QPointer<QtMainWindow> mainWindow;
    DAVA::TArc::WindowKey windowKey;

    DAVA_VIRTUAL_REFLECTION(REGlobalData)
    {
    };
};
}

REModule::~REModule()
{
    ControlsFactory::ReleaseFonts();
    GetAccessor().GetGlobalContext().DeleteData<REModuleDetail::REGlobalData>();
}

void REModule::OnRenderSystemInitialized(DAVA::Window* w)
{
    DAVA::Renderer::SetDesiredFPS(60);
    DAVA::DynamicBufferAllocator::SetPageSize(16 * 1024 * 1024); // 16 mb

    using TData = REModuleDetail::REGlobalData;
    DVASSERT(GetAccessor().GetGlobalContext().HasData<TData>());
    REModuleDetail::REGlobalData& globalData = GetAccessor().GetGlobalContext().GetData<REModuleDetail::REGlobalData>();
    globalData.mainWindow->OnRenderingInitialized();

    QObject::connect(globalData.launcher, &ResourceEditorLauncher::LaunchFinished, [this]()
                     {
                         REModuleDetail::REGlobalData& globalData = GetAccessor().GetGlobalContext().GetData<REModuleDetail::REGlobalData>();
                         globalData.mainWindow->SetupTitle();
                         globalData.mainWindow->OnSceneNew();
                     });
    globalData.launcher->Launch();
}

bool REModule::CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key)
{
    REModuleDetail::REGlobalData& globalData = GetAccessor().GetGlobalContext().GetData<REModuleDetail::REGlobalData>();
    DVASSERT(globalData.windowKey == key);
    bool hasChangedScenes = false;
    globalData.mainWindow->ForEachScene([&hasChangedScenes](SceneEditor2* scene)
                                        {
                                            hasChangedScenes |= scene->IsChanged();
                                        });

    return !hasChangedScenes;
}

bool REModule::ControlWindowClosing(const DAVA::TArc::WindowKey& key, QCloseEvent* event)
{
    REModuleDetail::REGlobalData& globalData = GetAccessor().GetGlobalContext().GetData<REModuleDetail::REGlobalData>();
    DVASSERT(globalData.windowKey == key);
    if (globalData.mainWindow->CanBeClosed())
    {
        globalData.mainWindow->CloseAllScenes();
        event->accept();
    }
    else
    {
        event->ignore();
    }

    return true;
}

void REModule::SaveOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void REModule::RestoreOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void REModule::OnContextCreated(DAVA::TArc::DataContext& context)
{
    DVASSERT(false, "Temporary assert. Now nobody creates context");
}

void REModule::OnContextDeleted(DAVA::TArc::DataContext& context)
{
    DVASSERT(false, "Temporary assert. Now nobody creates context and there is no context that can be deleted");
}

void REModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    REModuleDetail::REGlobalData& globalData = GetAccessor().GetGlobalContext().GetData<REModuleDetail::REGlobalData>();
    DVASSERT(globalData.windowKey == key);
    globalData.mainWindow->CloseAllScenes();
}

void REModule::PostInit()
{
    UnpackHelpDoc();

    Themes::InitFromQApplication();
    DAVA::TArc::ContextAccessor& accessor = GetAccessor();

    DAVA::EngineContext& engineContext = accessor.GetEngineContext();
    engineContext.localizationSystem->InitWithDirectory("~res:/Strings/");
    engineContext.localizationSystem->SetCurrentLocale("en");
    engineContext.uiControlSystem->SetClearColor(DAVA::Color(.3f, .3f, .3f, 1.f));

    using TData = REModuleDetail::REGlobalData;
    DAVA::TArc::DataContext& globalContext = accessor.GetGlobalContext();
    globalContext.CreateData(std::make_unique<TData>());
    TData& globalData = globalContext.GetData<TData>();

    DAVA::TArc::UIManager& ui = static_cast<DAVA::TArc::UIManager&>(GetUI());
    globalData.mainWindow->InjectRenderWidget(GetContextManager().GetRenderWidget());
    globalData.mainWindow->EnableGlobalTimeout(true);
    ui.InjectWindow(globalData.windowKey, globalData.mainWindow);
}

void REModule::UnpackHelpDoc()
{
    DAVA::EngineContext& engineContext = GetAccessor().GetEngineContext();
    DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
    DAVA::FilePath docsPath = DAVA::FilePath(ResourceEditor::DOCUMENTATION_PATH);
    if (editorVer != APPLICATION_BUILD_VERSION || !engineContext.fileSystem->Exists(docsPath))
    {
        DAVA::Logger::FrameworkDebug("Unpacking Help...");
        try
        {
            DAVA::ResourceArchive helpRA("~res:/Help.docs");
            engineContext.fileSystem->DeleteDirectory(docsPath);
            engineContext.fileSystem->CreateDirectory(docsPath, true);
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
