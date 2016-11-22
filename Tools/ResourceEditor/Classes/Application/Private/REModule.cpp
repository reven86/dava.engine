#include "Classes/Application/REModule.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Application/LaunchModuleData.h"

#include "Main/mainwindow.h"
#include "TextureCache.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "Deprecated/ControlsFactory.h"
#include "version.h"

#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/DataProcessing/DataNode.h"

#include "Debug/DVAssert.h"

#include <QPointer>

namespace REModuleDetail
{
class REGlobalData : public DAVA::TArc::DataNode
{
public:
    REGlobalData(DAVA::TArc::UI* ui)
    {
        textureCache = new TextureCache();
        mainWindow = new QtMainWindow(ui);
    }

    ~REGlobalData()
    {
        DAVA::SafeRelease(textureCache);

        if (!mainWindow.isNull())
        {
            QtMainWindow* mainWindowPointer = mainWindow.data();
            mainWindow.clear();
            DAVA::SafeDelete(mainWindowPointer);
        }
    }

    TextureCache* textureCache = nullptr;
    QPointer<QtMainWindow> mainWindow;

    DAVA_VIRTUAL_REFLECTION(REGlobalData)
    {
    };
};
}

REModule::~REModule()
{
    ControlsFactory::ReleaseFonts();
    GetAccessor()->GetGlobalContext()->DeleteData<REModuleDetail::REGlobalData>();
}

//void REModule::OnRenderSystemInitialized(DAVA::Window* w)
//{
//    using TData = REModuleDetail::REGlobalData;
//
//    using namespace DAVA::TArc;
//    ContextAccessor& accessor = GetAccessor();
//    DataContext* globalContext = accessor.GetGlobalContext();
//
//    REModuleDetail::REGlobalData* globalData = globalContext->GetData<REModuleDetail::REGlobalData>();
//    DVASSERT(globalData != nullptr);
//    globalData->mainWindow->OnRenderingInitialized();
//
//    launchDataWrapper = accessor.CreateWrapper(DAVA::ReflectedType::Get<LaunchModuleData>());
//    launchDataWrapper.SetListener(this);
//}
//
//bool REModule::CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key)
//{
//    using namespace DAVA::TArc;
//    ContextAccessor& accessor = GetAccessor();
//    DataContext* globalContext = accessor.GetGlobalContext();
//    REModuleDetail::REGlobalData* globalData = globalContext->GetData<REModuleDetail::REGlobalData>();
//    DVASSERT(globalData->windowKey == key);
//    bool hasChangedScenes = false;
//    globalData->mainWindow->ForEachScene([&hasChangedScenes](SceneEditor2* scene)
//                                         {
//                                             hasChangedScenes |= scene->IsChanged();
//                                         });
//
//    return !hasChangedScenes;
//}
//
//bool REModule::ControlWindowClosing(const DAVA::TArc::WindowKey& key, QCloseEvent* event)
//{
//    REModuleDetail::REGlobalData* globalData = GetAccessor().GetGlobalContext()->GetData<REModuleDetail::REGlobalData>();
//    DVASSERT(globalData->windowKey == key);
//    if (globalData->mainWindow->CanBeClosed())
//    {
//        globalData->mainWindow->CloseAllScenes();
//        event->accept();
//    }
//    else
//    {
//        event->ignore();
//    }
//
//    return true;
//}

void REModule::PostInit()
{
    Themes::InitFromQApplication();
    DAVA::TArc::ContextAccessor* accessor = GetAccessor();

    DAVA::EngineContext* engineContext = accessor->GetEngineContext();
    engineContext->localizationSystem->InitWithDirectory("~res:/Strings/");
    engineContext->localizationSystem->SetCurrentLocale("en");
    engineContext->uiControlSystem->SetClearColor(DAVA::Color(.3f, .3f, .3f, 1.f));

    using TData = REModuleDetail::REGlobalData;
    DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<TData>(GetUI()));
    TData* globalData = globalContext->GetData<TData>();

    DAVA::TArc::UIManager* ui = static_cast<DAVA::TArc::UIManager*>(GetUI());
    ui->InjectWindow(REGlobal::MainWindowKey, globalData->mainWindow);
}

void REModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA::TArc;
    DataContext* ctx = GetAccessor()->GetGlobalContext();
    LaunchModuleData* data = ctx->GetData<LaunchModuleData>();
    DVASSERT(data != nullptr);
    if (!data->IsLaunchFinished())
    {
        return;
    }

    REModuleDetail::REGlobalData* globalData = ctx->GetData<REModuleDetail::REGlobalData>();
    // TODO UVR LATER
    //globalData->mainWindow->OnSceneNew();

    launchDataWrapper.SetListener(nullptr);
    launchDataWrapper = DAVA::TArc::DataWrapper();
}
