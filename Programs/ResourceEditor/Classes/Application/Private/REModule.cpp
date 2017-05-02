#include "Classes/Application/REModule.h"
#include "Classes/Application/REGlobal.h"

#include "Main/mainwindow.h"
#include "TextureCache.h"
#include "QtTools/Utils/Themes/Themes.h"

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

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(REGlobalData)
    {
    };
};
}

REModule::~REModule()
{
    GetAccessor()->GetGlobalContext()->DeleteData<REModuleDetail::REGlobalData>();
}

void REModule::PostInit()
{
    Themes::InitFromQApplication();
    DAVA::TArc::ContextAccessor* accessor = GetAccessor();

    const DAVA::EngineContext* engineContext = accessor->GetEngineContext();
    engineContext->localizationSystem->InitWithDirectory("~res:/Strings/");
    engineContext->localizationSystem->SetCurrentLocale("en");
    engineContext->uiControlSystem->SetClearColor(DAVA::Color(.3f, .3f, .3f, 1.f));

    using TData = REModuleDetail::REGlobalData;
    DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<TData>(GetUI()));
    TData* globalData = globalContext->GetData<TData>();

    DAVA::TArc::UI* ui = GetUI();
    ui->InjectWindow(DAVA::TArc::mainWindowKey, globalData->mainWindow);
    globalData->mainWindow->AfterInjectInit();
    globalData->mainWindow->EnableGlobalTimeout(true);

    RegisterOperation(REGlobal::ShowMaterial.ID, this, &REModule::ShowMaterial);
}

void REModule::ShowMaterial(DAVA::NMaterial* material)
{
    using TData = REModuleDetail::REGlobalData;
    DAVA::TArc::DataContext* globalContext = GetAccessor()->GetGlobalContext();
    TData* globalData = globalContext->GetData<TData>();
    globalData->mainWindow->OnMaterialEditor(material);
}
