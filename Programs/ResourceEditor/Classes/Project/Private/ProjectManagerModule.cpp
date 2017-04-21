#include "Classes/Project/ProjectManagerModule.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Project/ProjectResources.h"

#include "Classes/Application/REGlobal.h"
#include "Classes/Settings/Settings.h"
#include "Classes/Settings/SettingsManager.h"
#include "Classes/Qt/SpritesPacker/SpritesPackerModule.h"
#include "Deprecated/EditorConfig.h"

#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/QtAction.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Engine/EngineContext.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FileSystem.h"

#include <QAction>

namespace ProjectManagerDetails
{
const DAVA::String PROPERTIES_KEY = "ProjectManagerProperties";
}

ProjectManagerModule::~ProjectManagerModule() = default;

void ProjectManagerModule::PostInit()
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();

    projectResources.reset(new ProjectResources(accessor));

    ProjectManagerData* data = GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>();
    data->spritesPacker.reset(new SpritesPackerModule(GetUI()));
    data->editorConfig.reset(new EditorConfig());

    CreateActions();
    RegisterOperations();

    RecentMenuItems::Params params(DAVA::TArc::mainWindowKey, accessor, "Recent projects");
    params.ui = GetUI();
    params.getMaximumCount = []() {
        return SettingsManager::GetValue(Settings::General_RecentProjectsCount).AsInt32();
    };

    params.menuSubPath << MenuItems::menuFile
                       << "Recent Projects";
    params.insertionParams.method = InsertionParams::eInsertionMethod::BeforeItem;

    recentProjects.reset(new RecentMenuItems(std::move(params)));
    recentProjects->actionTriggered.Connect([this](const DAVA::String& projectPath)
                                            {
                                                OpenProjectByPath(DAVA::FilePath(projectPath));
                                            });
}

void ProjectManagerModule::CreateActions()
{
    using namespace DAVA::TArc;
    UI* ui = GetUI();

    const QString openProjectName("Open Project");
    const QString recentProjectsName("Recent Projects");
    const QString closeProjectsName("Close Project");

    // OpenProject action
    {
        QAction* openProjectAction = new QAction(QIcon(":/QtIcons/openproject.png"), openProjectName, nullptr);
        connections.AddConnection(openProjectAction, &QAction::triggered, [this]()
                                  {
                                      OpenProject();
                                  });

        ActionPlacementInfo placementInfo(CreateMenuPoint(MenuItems::menuFile, InsertionParams(InsertionParams::eInsertionMethod::BeforeItem)));
        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, openProjectAction);
    }

    // RecentProjects
    {
        QAction* recentProjects = new QAction(recentProjectsName, nullptr);
        ActionPlacementInfo placementInfo(CreateMenuPoint(MenuItems::menuFile, InsertionParams(InsertionParams::eInsertionMethod::AfterItem, openProjectName)));
        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, recentProjects);
    }

    {
        QtAction* closeProjectAction = new QtAction(GetAccessor(), closeProjectsName, nullptr);

        FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectManagerData>();
        fieldDescr.fieldName = DAVA::FastName(ProjectManagerData::ProjectPathProperty);
        closeProjectAction->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& fieldValue) -> DAVA::Any {
            return fieldValue.CanCast<DAVA::FilePath>() && !fieldValue.Cast<DAVA::FilePath>().IsEmpty();
        });
        connections.AddConnection(closeProjectAction, &QAction::triggered, [this]() {
            CloseProject();
        });
        ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint(MenuItems::menuFile, InsertionParams(InsertionParams::eInsertionMethod::AfterItem, recentProjectsName)));
        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, closeProjectAction);
    }

    // Separator
    {
        QAction* separator = new QAction(nullptr);
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo(CreateMenuPoint(MenuItems::menuFile, InsertionParams(InsertionParams::eInsertionMethod::AfterItem, closeProjectsName)));
        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, separator);
    }

    InsertionParams reloadSpritesInsertionParams;
    reloadSpritesInsertionParams.method = InsertionParams::eInsertionMethod::BeforeItem;
    ActionPlacementInfo reloadSpritePlacement(CreateToolbarPoint("sceneToolBar", reloadSpritesInsertionParams));
    QAction* reloadSprites = new QAction(QIcon(":/QtIcons/refresh_particle.png"), "Reload Sprites", nullptr);
    connections.AddConnection(reloadSprites, &QAction::triggered, [this]()
                              {
                                  ReloadSprites();
                              });
    ui->AddAction(mainWindowKey, reloadSpritePlacement, reloadSprites);
}

void ProjectManagerModule::RegisterOperations()
{
    RegisterOperation(REGlobal::OpenLastProjectOperation.ID, this, &ProjectManagerModule::OpenLastProject);
}

void ProjectManagerModule::OpenProject()
{
    DAVA::TArc::DirectoryDialogParams dirDialogParams;
    dirDialogParams.title = QString("Open Project Folder");
    QString dirPath = GetUI()->GetExistingDirectory(DAVA::TArc::mainWindowKey, dirDialogParams);
    if (!dirPath.isEmpty())
    {
        DAVA::FilePath path(dirPath.toStdString());
        path.MakeDirectoryPathname();
        OpenProjectByPath(path);
    }
}

void ProjectManagerModule::OpenProjectByPath(const DAVA::FilePath& incomePath)
{
    ProjectManagerData* data = GetData();

    if (incomePath.IsDirectoryPathname() && incomePath != data->projectPath)
    {
        bool closed = CloseProject();
        if (closed == false)
        {
            return;
        }

        DAVA::FileSystem* fileSystem = GetAccessor()->GetEngineContext()->fileSystem;
        if (fileSystem->Exists(incomePath))
        {
            bool reloadParticles = SettingsManager::GetValue(Settings::General_ReloadParticlesOnPojectOpening).AsBool();
            DVASSERT(data->spritesPacker != nullptr);
            if (reloadParticles)
            {
                auto func = DAVA::Bind(&ProjectManagerModule::OpenProjectImpl, this, incomePath);
                connections.AddConnection(data->spritesPacker.get(), &SpritesPackerModule::SpritesReloaded, func, Qt::QueuedConnection);
                data->spritesPacker->RepackImmediately(incomePath, static_cast<DAVA::eGPUFamily>(SettingsManager::GetValue(Settings::Internal_SpriteViewGPU).AsUInt32()));
            }
            else
            {
                OpenProjectImpl(incomePath);
            }
        }
    }
}

void ProjectManagerModule::OpenProjectImpl(const DAVA::FilePath& incomePath)
{
    ProjectManagerData* data = GetData();
    connections.RemoveConnection(data->spritesPacker.get(), &SpritesPackerModule::SpritesReloaded);

    const DAVA::EngineContext* engineCtx = GetAccessor()->GetEngineContext();
    DAVA::FileSystem* fileSystem = engineCtx->fileSystem;

    projectResources->LoadProject(incomePath);
    DAVA::TArc::PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(ProjectManagerDetails::PROPERTIES_KEY);

    DAVA::FilePath editorConfigPath = data->GetProjectPath() + "EditorConfig.yaml";
    if (fileSystem->Exists(editorConfigPath))
    {
        data->editorConfig->ParseConfig(editorConfigPath);
    }
    else
    {
        DAVA::Logger::Warning("Selected project doesn't contains EditorConfig.yaml");
    }

    propsItem.Set(Settings::Internal_LastProjectPath.c_str(), DAVA::Any(data->projectPath));

    recentProjects->Add(incomePath.GetAbsolutePathname());
}

void ProjectManagerModule::OpenLastProject()
{
    DAVA::FilePath projectPath;
    {
        DAVA::TArc::PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(ProjectManagerDetails::PROPERTIES_KEY);
        projectPath = propsItem.Get<DAVA::FilePath>(Settings::Internal_LastProjectPath.c_str());
    }

    if (projectPath.IsEmpty())
    {
        // Back compatibility
        projectPath = SettingsManager::Instance()->GetValue(Settings::Internal_LastProjectPath).AsFilePath();
        SettingsManager::Instance()->SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(DAVA::FilePath()));
    }

    if (!projectPath.IsEmpty())
    {
        DVASSERT(projectPath.IsDirectoryPathname());
        OpenProjectByPath(projectPath);
    }
}

bool ProjectManagerModule::CloseProject()
{
    ProjectManagerData* data = GetData();
    if (!data->projectPath.IsEmpty())
    {
        InvokeOperation(REGlobal::CloseAllScenesOperation.ID, true);
        if (GetAccessor()->GetContextCount() != 0)
        {
            return false;
        }

        projectResources->UnloadProject();

        SettingsManager::ResetPerProjectSettings();
        DAVA::TArc::PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(ProjectManagerDetails::PROPERTIES_KEY);
        propsItem.Set(Settings::Internal_LastProjectPath.c_str(), DAVA::Any(DAVA::FilePath()));
    }

    return true;
}

void ProjectManagerModule::ReloadSprites()
{
    using namespace DAVA::TArc;

    DataContext* ctx = GetAccessor()->GetGlobalContext();
    ProjectManagerData* data = ctx->GetData<ProjectManagerData>();
    DVASSERT(data);
    data->spritesPacker->RepackWithDialog();
}

ProjectManagerData* ProjectManagerModule::GetData()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    return ctx->GetData<ProjectManagerData>();
}
