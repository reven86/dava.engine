#include "Classes/Project/ProjectManagerModule.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Project/ProjectResources.h"

#include "Classes/Application/REGlobal.h"
#include "Classes/Qt/Settings/Settings.h"
#include "Classes/Qt/SpritesPacker/SpritesPackerModule.h"
#include "Classes/Qt/Settings/SettingsManager.h"
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

    RecentMenuItems::Params params;
    params.accessor = accessor;
    params.ui = GetUI();
    params.settingsKeyCount = Settings::General_RecentProjectsCount;
    params.settingsKeyData = Settings::Internal_RecentProjects;
    params.menuSubPath << "File"
                       << "Recent Projects";
    params.insertionParams.method = InsertionParams::eInsertionMethod::BeforeItem;

    recentProject.reset(new RecentMenuItems(params));
    recentProject->actionTriggered.ConnectDetached([this](const DAVA::String& projectPath)
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

        DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint("File", DAVA::TArc::InsertionParams(InsertionParams::eInsertionMethod::BeforeItem)));
        ui->AddAction(REGlobal::MainWindowKey, placementInfo, openProjectAction);
    }

    // RecentProjects
    {
        QAction* recentProjects = new QAction(recentProjectsName, nullptr);
        DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint("File", DAVA::TArc::InsertionParams(InsertionParams::eInsertionMethod::AfterItem, openProjectName)));
        ui->AddAction(REGlobal::MainWindowKey, placementInfo, recentProjects);
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
        DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint("File", DAVA::TArc::InsertionParams(InsertionParams::eInsertionMethod::AfterItem, recentProjectsName)));
        ui->AddAction(REGlobal::MainWindowKey, placementInfo, closeProjectAction);
    }

    // Separator
    {
        QAction* separator = new QAction(nullptr);
        separator->setSeparator(true);
        DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint("File", DAVA::TArc::InsertionParams(InsertionParams::eInsertionMethod::AfterItem, closeProjectsName)));
        ui->AddAction(REGlobal::MainWindowKey, placementInfo, separator);
    }

    DAVA::TArc::InsertionParams reloadSpritesInsertionParams;
    reloadSpritesInsertionParams.method = InsertionParams::eInsertionMethod::BeforeItem;
    DAVA::TArc::ActionPlacementInfo reloadSpritePlacement(DAVA::TArc::CreateToolbarPoint("sceneToolBar", reloadSpritesInsertionParams));
    QAction* reloadSprites = new QAction(QIcon(":/QtIcons/refresh_particle.png"), "Reload Sprites", nullptr);
    connections.AddConnection(reloadSprites, &QAction::triggered, [this]()
                              {
                                  ReloadSprites();
                              });
    ui->AddAction(REGlobal::MainWindowKey, reloadSpritePlacement, reloadSprites);
}

void ProjectManagerModule::RegisterOperations()
{
    RegisterOperation(REGlobal::OpenLastProjectOperation.ID, this, &ProjectManagerModule::OpenLastProject);
}

void ProjectManagerModule::OpenProject()
{
    DAVA::TArc::DirectoryDialogParams dirDialogParams;
    dirDialogParams.title = QString("Open Project Folder");
    QString dirPath = GetUI()->GetExistingDirectory(REGlobal::MainWindowKey, dirDialogParams);
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

    recentProject->Add(incomePath.GetAbsolutePathname());
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
