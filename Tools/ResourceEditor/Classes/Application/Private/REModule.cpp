#include "Classes/Application/REModule.h"
#include "Classes/Application/REGlobal.h"

#include "Main/mainwindow.h"
#include "TextureCache.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "Deprecated/ControlsFactory.h"

#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/DataProcessing/DataNode.h"

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

void REModule::PostInit()
{
    Themes::InitFromQApplication();
    DAVA::TArc::ContextAccessor* accessor = GetAccessor();

    const DAVA::EngineContext* engineContext = accessor.GetEngineContext();
    engineContext->localizationSystem->InitWithDirectory("~res:/Strings/");
    engineContext->localizationSystem->SetCurrentLocale("en");
    engineContext->uiControlSystem->SetClearColor(DAVA::Color(.3f, .3f, .3f, 1.f));

    using TData = REModuleDetail::REGlobalData;
    DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<TData>(GetUI()));
    TData* globalData = globalContext->GetData<TData>();

    DAVA::TArc::UIManager* ui = static_cast<DAVA::TArc::UIManager*>(GetUI());
    ui->InjectWindow(REGlobal::MainWindowKey, globalData->mainWindow);
    globalData->mainWindow->EnableGlobalTimeout(true);
}
