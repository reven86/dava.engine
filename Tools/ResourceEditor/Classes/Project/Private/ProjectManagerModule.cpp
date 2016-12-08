#include "Classes/Project/ProjectManagerModule.h"
#include "Classes/Project/ProjectManagerData.h"

#include "Classes/Application/REGlobal.h"
#include "Classes/Qt/Settings/Settings.h"
#include "Classes/Qt/SpritesPacker/SpritesPackerModule.h"
#include "Classes/Qt/Settings/SettingsManager.h"
#include "Classes/Qt/Main/QtUtils.h"
#include "Deprecated/EditorConfig.h"

#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/UI.h"

#include "QtTools/ProjectInformation/FileSystemCache.h"

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

DAVA::TArc::WindowKey GetREWidnowKey()
{
    return DAVA::TArc::WindowKey(REGlobal::MainWindowName);
}
}

void ProjectManagerModule::OnContextCreated(DAVA::TArc::DataContext& context)
{
}

void ProjectManagerModule::OnContextDeleted(DAVA::TArc::DataContext& context)
{
}

void ProjectManagerModule::PostInit()
{
    using namespace DAVA::TArc;

    ContextAccessor& accessor = GetAccessor();

    DataContext* globalContext = accessor.GetGlobalContext();
    std::unique_ptr<ProjectManagerData> data = std::make_unique<ProjectManagerData>();

    QStringList extensions = { "sc2" };
    data->dataSourceSceneFiles.reset(new FileSystemCache(extensions));
    data->spritesPacker.reset(new SpritesPackerModule(&GetUI()));
    data->editorConfig.reset(new EditorConfig());
    globalContext->CreateData(std::move(data));

    CreateActions();
    RegisterOperations();
}

void ProjectManagerModule::CreateActions()
{
    using namespace DAVA::TArc;
    UI& ui = GetUI();
    DAVA::TArc::WindowKey windowKey = ProjectManagerDetails::GetREWidnowKey();

    DAVA::TArc::InsertionParams insertionParams;
    insertionParams.method = DAVA::TArc::InsertionParams::eInsertionMethod::BeforeItem;
    insertionParams.item = QString("actionNewScene");
    DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint("File", insertionParams));

    QAction* openProjectAction = new QAction(QIcon(":/QtIcons/openproject.png"), "Open Project", nullptr);
    connections.AddConnection(openProjectAction, &QAction::triggered, [this]()
                              {
                                  OpenProject();
                              });
    ui.AddAction(windowKey, placementInfo, openProjectAction);

    QAction* recentProjects = new QAction("Recent Projects", nullptr);
    ui.AddAction(windowKey, placementInfo, recentProjects);

    QAction* closeProjectAction = new QAction("Close Project", nullptr);
    closeProjectAction->setEnabled(false);
    connections.AddConnection(closeProjectAction, &QAction::triggered, [this]()
                              {
                                  CloseProject();
                              });
    ui.AddAction(windowKey, placementInfo, closeProjectAction);
    closeAction = closeProjectAction;

    QAction* separator = new QAction(nullptr);
    separator->setSeparator(true);
    ui.AddAction(windowKey, placementInfo, separator);

    DAVA::TArc::InsertionParams reloadSpritesInsertionParams;
    reloadSpritesInsertionParams.method = InsertionParams::eInsertionMethod::BeforeItem;
    DAVA::TArc::ActionPlacementInfo reloadSpritePlacement(DAVA::TArc::CreateToolbarPoint("sceneToolBar", reloadSpritesInsertionParams));
    QAction* reloadSprites = new QAction(QIcon(":/QtIcons/refresh_particle.png"), "Reload Sprites", nullptr);
    connections.AddConnection(reloadSprites, &QAction::triggered, [this]()
                              {
                                  ReloadSprites();
                              });
    ui.AddAction(windowKey, reloadSpritePlacement, reloadSprites);

    AddRecentProjectActions();
}

void ProjectManagerModule::RegisterOperations()
{
    RegisterOperation(REGlobal::OpenLastProjectOperation.ID, this, &ProjectManagerModule::OpenLastProject);
}

void ProjectManagerModule::OpenProject()
{
    DAVA::TArc::DirectoryDialogParams dirDialogParams;
    dirDialogParams.title = QString("Open Project Folder");
    QString dirPath = GetUI().GetExistingDirectory(DAVA::TArc::WindowKey(REGlobal::MainWindowName), dirDialogParams);
    if (!dirPath.isEmpty())
    {
        DAVA::FilePath path = DAVA::FilePath(PathnameToDAVAStyle(dirPath));
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

        DAVA::FileSystem* fileSystem = GetAccessor().GetEngineContext()->fileSystem;
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
    connections.disconnect(data->spritesPacker.get(), &SpritesPackerModule::SpritesReloaded, nullptr, nullptr);
    data->projectPath = incomePath;
    DAVA::FilePath::AddTopResourcesFolder(data->GetDataPath());

    DAVA::TArc::PropertiesItem propsItem = GetAccessor().CreatePropertiesNode(ProjectManagerDetails::PROPERTIES_KEY);

    propsItem.Set(Settings::Internal_LastProjectPath.c_str(), DAVA::Any(data->projectPath));
    LoadMaterialsSettings(data);

    DAVA::QualitySettingsSystem::Instance()->Load("~res:/quality.yaml");
    const DAVA::EngineContext* engineCtx = GetAccessor().GetEngineContext();
    engineCtx->soundSystem->InitFromQualitySettings();

    DAVA::FileSystem* fileSystem = engineCtx->fileSystem;
    fileSystem->CreateDirectory(data->GetWorkspacePath(), true);
    if (fileSystem->Exists(data->GetDataSourcePath()))
    {
        data->dataSourceSceneFiles->TrackDirectory(QString::fromStdString(data->GetDataSourcePath().GetStringValue()));
    }

    data->editorConfig->ParseConfig(data->GetProjectPath() + "EditorConfig.yaml");

    AddRecentProject(incomePath);

    // TODO UVR remove imperative code that sync state of action
    if (closeAction)
    {
        closeAction->setEnabled(true);
    }
}

void ProjectManagerModule::OpenLastProject()
{
    ProjectManagerData* data = GetData();

    DAVA::TArc::PropertiesItem propsItem = GetAccessor().CreatePropertiesNode(ProjectManagerDetails::PROPERTIES_KEY);
    DAVA::FilePath projectPath = propsItem.Get<DAVA::FilePath>(Settings::Internal_LastProjectPath.c_str());
    if (projectPath.IsEmpty())
    {
        // Back compatibility
        projectPath = SettingsManager::Instance()->GetValue(Settings::Internal_LastProjectPath).AsFilePath();
        SettingsManager::Instance()->SetValue(Settings::Internal_LastProjectPath, DAVA::VariantType(DAVA::FilePath()));
    }

    if (!projectPath.IsEmpty())
    {
        DVASSERT(projectPath.IsDirectoryPathname());
        delayedExecutor.DelayedExecute(DAVA::Bind(&ProjectManagerModule::OpenProjectByPath, this, projectPath));
    }
}

bool ProjectManagerModule::CloseProject()
{
    ProjectManagerData* data = GetData();

    if (!data->projectPath.IsEmpty())
    {
        DVASSERT(data->closeProjectPredicate != nullptr);
        if (data->closeProjectPredicate() == false)
        {
            return false;
        }

        // TODO UVR remove imperative code that sync state of action
        if (closeAction)
        {
            closeAction->setEnabled(false);
        }
        DAVA::FilePath::RemoveResourcesFolder(data->GetDataPath());
        data->dataSourceSceneFiles->UntrackDirectory(QString::fromStdString(data->GetDataSourcePath().GetStringValue()));
        data->projectPath = "";

        SettingsManager::ResetPerProjectSettings();
        DAVA::TArc::PropertiesItem propsItem = GetAccessor().CreatePropertiesNode(ProjectManagerDetails::PROPERTIES_KEY);
        propsItem.Set(Settings::Internal_LastProjectPath.c_str(), DAVA::Any(DAVA::FilePath()));
    }

    return true;
}

void ProjectManagerModule::ReloadSprites()
{
    using namespace DAVA::TArc;

    DataContext* ctx = GetAccessor().GetGlobalContext();
    ProjectManagerData* data = ctx->GetData<ProjectManagerData>();
    DVASSERT(data);
    data->spritesPacker->RepackWithDialog();
}

void ProjectManagerModule::LoadMaterialsSettings(ProjectManagerData* data)
{
    data->templates.clear();
    data->qualities.clear();

    // parse available material templates
    const DAVA::FilePath materialsListPath = DAVA::FilePath("~res:/Materials/assignable.yaml");
    DAVA::FileSystem* fileSystem = GetAccessor().GetEngineContext()->fileSystem;
    if (fileSystem->Exists(materialsListPath))
    {
        DAVA::ScopedPtr<DAVA::YamlParser> parser(DAVA::YamlParser::Create(materialsListPath));
        DAVA::YamlNode* rootNode = parser->GetRootNode();

        if (nullptr != rootNode)
        {
            DAVA::FilePath materialsListDir = materialsListPath.GetDirectory();

            for (DAVA::uint32 i = 0; i < rootNode->GetCount(); ++i)
            {
                const DAVA::YamlNode* templateNode = rootNode->Get(i);
                if (nullptr != templateNode)
                {
                    const DAVA::YamlNode* name = templateNode->Get("name");
                    const DAVA::YamlNode* path = templateNode->Get("path");

                    if (nullptr != name && nullptr != path &&
                        name->GetType() == DAVA::YamlNode::TYPE_STRING &&
                        path->GetType() == DAVA::YamlNode::TYPE_STRING)
                    {
                        const DAVA::FilePath templatePath = materialsListDir + path->AsString();
                        if (fileSystem->Exists(templatePath))
                        {
                            ProjectManagerData::AvailableMaterialTemplate amt;
                            amt.name = name->AsString().c_str();
                            amt.path = templatePath.GetFrameworkPath().c_str();

                            data->templates.append(amt);
                        }
                    }
                }
            }
        }
    }
}

ProjectManagerData* ProjectManagerModule::GetData()
{
    using namespace DAVA::TArc;
    ContextAccessor& accessor = GetAccessor();
    DataContext* ctx = accessor.GetGlobalContext();
    return ctx->GetData<ProjectManagerData>();
}

void ProjectManagerModule::AddRecentProjectActions()
{
    using namespace DAVA::TArc;

    UI& ui = GetUI();
    DAVA::Vector<DAVA::String> recentProjects = GetRecentProjects();

    WindowKey windowKey = ProjectManagerDetails::GetREWidnowKey();
    for (DAVA::String& projectPath : recentProjects)
    {
        QAction* project = new QAction(QString::fromStdString(projectPath), nullptr);
        connections.AddConnection(project, &QAction::triggered, [this, projectPath]()
                                  {
                                      OpenProjectByPath(projectPath);
                                  });

        ActionPlacementInfo placement(CreateMenuPoint(QString::fromStdString(DAVA::String("File$/Recent Projects"))));
        ui.AddAction(windowKey, placement, project);
    }
}

void ProjectManagerModule::AddRecentProject(const DAVA::FilePath& projectPath)
{
    RemoveRecentProjects();

    DAVA::Vector<DAVA::String> vectorToSave = GetRecentProjects();

    DAVA::String stringToInsert = projectPath.GetAbsolutePathname();

    //check present set to avoid duplicates
    vectorToSave.erase(std::remove(vectorToSave.begin(), vectorToSave.end(), stringToInsert), vectorToSave.end());
    vectorToSave.insert(vectorToSave.begin(), stringToInsert);

    DAVA::uint32 recentFilesMaxCount = SettingsManager::GetValue(Settings::General_RecentProjectsCount).AsInt32();
    DAVA::uint32 size = DAVA::Min((DAVA::uint32)vectorToSave.size(), recentFilesMaxCount);

    DAVA::KeyedArchive* archive = new DAVA::KeyedArchive();
    for (DAVA::uint32 i = 0; i < size; ++i)
    {
        archive->SetString(DAVA::Format("%d", i), vectorToSave[i]);
    }
    SettingsManager::SetValue(Settings::Internal_RecentProjects, DAVA::VariantType(archive));
    SafeRelease(archive);

    AddRecentProjectActions();
}

void ProjectManagerModule::RemoveRecentProjects()
{
    using namespace DAVA::TArc;

    UI& ui = GetUI();
    DAVA::Vector<DAVA::String> recentProjects = GetRecentProjects();

    WindowKey windowKey = ProjectManagerDetails::GetREWidnowKey();
    for (DAVA::String& projectPath : recentProjects)
    {
        ui.RemoveAction(windowKey, ActionPlacementInfo(CreateMenuPoint(QString::fromStdString(DAVA::String("File$/Recent Projects$/") + projectPath))));
    }
}

DAVA::Vector<DAVA::String> ProjectManagerModule::GetRecentProjects()
{
    DAVA::Vector<DAVA::String> retVector;
    DAVA::VariantType recentFilesVariant = SettingsManager::GetValue(Settings::Internal_RecentProjects);
    if (recentFilesVariant.GetType() == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
    {
        DAVA::KeyedArchive* archiveRecentFiles = recentFilesVariant.AsKeyedArchive();
        DAVA::uint32 recentFilesMaxCount = SettingsManager::GetValue(Settings::General_RecentProjectsCount).AsInt32();
        DAVA::uint32 size = DAVA::Min(archiveRecentFiles->Count(), recentFilesMaxCount);
        retVector.resize(size);
        for (DAVA::uint32 i = 0; i < size; ++i)
        {
            retVector[i] = archiveRecentFiles->GetString(DAVA::Format("%d", i));
        }
    }
    return retVector;
}
