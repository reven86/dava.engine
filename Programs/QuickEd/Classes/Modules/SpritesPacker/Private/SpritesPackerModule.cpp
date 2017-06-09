#include "Modules/SpritesPacker/SpritesPackerModule.h"
#include "Modules/SpritesPacker/SpritesPackerData.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Modules/LegacySupportModule/Private/Project.h"
#include "Application/QEGlobal.h"

#include <TArc/DataProcessing/DataContext.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>

#include <QtTools/ReloadSprites/SpritesPacker.h>
#include <QtTools/ReloadSprites/DialogReloadSprites.h>

#include <Logger/Logger.h>
#include <Render/2D/Sprite.h>
#include <FileSystem/FilePath.h>
#include <Preferences/PreferencesStorage.h>

#include <QDir>

REGISTER_PREFERENCES_ON_START(SpritesPackerModuleSettings,
                              PREF_ARG("isUsingAssetCache", false),
                              )

DAVA_VIRTUAL_REFLECTION_IMPL(SpritesPackerModule)
{
    DAVA::ReflectionRegistrator<SpritesPackerModule>::Begin()
    .ConstructorByPointer()
    .End();
}

SpritesPackerModule::SpritesPackerModule() = default;
SpritesPackerModule::~SpritesPackerModule() = default;

void SpritesPackerModule::PostInit()
{
    using namespace DAVA::TArc;

    std::unique_ptr<SpritesPackerData> spritesPackerData(new SpritesPackerData());
    connections.AddConnection(spritesPackerData->GetSpritesPacker(), &SpritesPacker::Finished, DAVA::MakeFunction(this, &SpritesPackerModule::OnReloadFinished));

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::move(spritesPackerData));

    settings.reset(new SpritesPackerModuleSettings(this));
    //we need to register preferences when whole class is initialized
    PreferencesStorage::Instance()->RegisterPreferences(settings.get());

    CreateActions();
}

void SpritesPackerModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    PreferencesStorage::Instance()->UnregisterPreferences(settings.get());
    settings.reset();
    GetAccessor()->GetGlobalContext()->DeleteData<SpritesPackerData>();
}

void SpritesPackerModule::CreateActions()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();

    QtAction* action = new QtAction(accessor, QIcon(":/Icons/reload.png"), QString("Reload Sprites"));
    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&SpritesPackerModule::OnReloadSprites, this));
    FieldDescriptor fieldDescr;
    fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectData>();
    fieldDescr.fieldName = DAVA::FastName(ProjectData::projectPathPropertyName);
    action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& fieldValue) -> DAVA::Any {
        return !fieldValue.Cast<DAVA::FilePath>(DAVA::FilePath()).IsEmpty();
    });

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint("Tools", { InsertionParams::eInsertionMethod::AfterItem }));
    placementInfo.AddPlacementPoint(CreateToolbarPoint("toolBarGlobal", { InsertionParams::eInsertionMethod::BeforeItem }));

    GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
}

bool SpritesPackerModule::IsUsingAssetCache() const
{
    const DAVA::TArc::ContextAccessor* accessor = GetAccessor();
    const DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    const SpritesPackerData* spritesPackerData = globalContext->GetData<SpritesPackerData>();
    return spritesPackerData->GetSpritesPacker()->IsUsingCache();
}

void SpritesPackerModule::SetUsingAssetCacheEnabled(bool enabled)
{
    if (enabled)
    {
        EnableCacheClient();
    }
    else
    {
        DisableCacheClient();
    }
}

void SpritesPackerModule::EnableCacheClient()
{
    using namespace DAVA;

    DisableCacheClient();
    cacheClient.reset(new AssetCacheClient());
    AssetCache::Error connected = cacheClient->ConnectSynchronously(connectionParams);
    if (connected != AssetCache::Error::NO_ERRORS)
    {
        cacheClient.reset();
        Logger::Warning("Asset cache client was not started! Error: %d", connected);
    }
    else
    {
        Logger::Info("Asset cache client started");
        SpritesPackerData* spritesPackerData = GetAccessor()->GetGlobalContext()->GetData<SpritesPackerData>();
        spritesPackerData->GetSpritesPacker()->SetCacheClient(cacheClient.get(), "QuickEd.ReloadSprites");
    }
}

void SpritesPackerModule::DisableCacheClient()
{
    if (cacheClient != nullptr && cacheClient->IsConnected())
    {
        cacheClient->Disconnect();
        cacheClient.reset();
        SpritesPackerData* spritesPackerData = GetAccessor()->GetGlobalContext()->GetData<SpritesPackerData>();
        spritesPackerData->GetSpritesPacker()->SetCacheClient(cacheClient.get(), "QuickEd.ReloadSprites");
    }
}

void SpritesPackerModule::OnReloadFinished()
{
    DAVA::Sprite::ReloadSprites();
}

void SpritesPackerModule::OnReloadSprites()
{
    using namespace DAVA;
    using namespace TArc;
    ContextAccessor* accessor = GetAccessor();
    InvokeOperation(QEGlobal::CloseAllDocuments.ID);
    if (accessor->GetContextCount() != 0)
    {
        return;
    }
    DataContext* globalContext = accessor->GetGlobalContext();
    SpritesPackerData* spritesPackerData = GetAccessor()->GetGlobalContext()->GetData<SpritesPackerData>();
    ProjectData* projectData = globalContext->GetData<ProjectData>();
    SpritesPacker* spritesPacker = spritesPackerData->GetSpritesPacker();
    DVASSERT(spritesPackerData != nullptr);
    DVASSERT(projectData != nullptr);
    DVASSERT(spritesPacker != nullptr);

    spritesPacker->ClearTasks();

    for (const auto& gfxOptions : projectData->GetGfxDirectories())
    {
        QDir gfxDirectory(QString::fromStdString(gfxOptions.directory.absolute.GetStringValue()));
        DVASSERT(gfxDirectory.exists());

        FilePath gfxOutDir = projectData->GetConvertedResourceDirectory().absolute + gfxOptions.directory.relative;
        QDir gfxOutDirectory(QString::fromStdString(gfxOutDir.GetStringValue()));

        spritesPacker->AddTask(gfxDirectory, gfxOutDirectory);
    }

    DialogReloadSprites dialogReloadSprites(spritesPacker, GetUI()->GetWindow(DAVA::TArc::mainWindowKey));
    dialogReloadSprites.exec();
}

SpritesPackerModuleSettings::SpritesPackerModuleSettings(SpritesPackerModule* module_)
    : module(module_)
{
}

bool SpritesPackerModuleSettings::IsUsingAssetCache() const
{
    return module->IsUsingAssetCache();
}

void SpritesPackerModuleSettings::SetUsingAssetCacheEnabled(bool enabled)
{
    module->SetUsingAssetCacheEnabled(enabled);
}

DECL_GUI_MODULE(SpritesPackerModule);
