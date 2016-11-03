#include "Classes/Qt/Application/ProjectManagerModule.h"

#include "Classes/Qt/Application/REGlobal.h"
#include "Classes/Qt/DataStructures/ProjectManagerData.h"
#include "Classes/Qt/Settings/Settings.h"
#include "Classes/Qt/SpritesPacker/SpritesPackerModule.h"

#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/UI.h"

#include "QtTools/ProjectInformation/ProjectStructure.h"

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

    DAVA::Vector<DAVA::String> extensions = { "sc2" };
    data->dataSourceSceneFiles.reset(new ProjectStructure(extensions));
    data->spritesPacker.reset(new SpritesPackerModule(&GetUI()));
    globalContext->CreateData(std::move(data));

    CreateActions();
    RegisterOperations();
}

void ProjectManagerModule::CreateActions()
{
    using namespace DAVA::TArc;
    UI& ui = GetUI();
    DAVA::TArc::WindowKey windowKey = ProjectManagerDetails::GetREWidnowKey();

    DAVA::TArc::MenuInsertionParams insertionParams;
    insertionParams.method = DAVA::TArc::MenuInsertionParams::eInsertionMethod::BeforeItem;
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

    DAVA::TArc::ActionPlacementInfo reloadSpritePlacement(DAVA::TArc::CreateToolbarPoint("sceneToolBar"));
    QAction* reloadSprites = new QAction(QIcon(":/QtIcons/refresh_particle.png"), "Reload Sprites", nullptr);
    connections.AddConnection(reloadSprites, &QAction::triggered, [this]()
                              {
                                  ReloadSprites();
                              });
    ui.AddAction(windowKey, reloadSpritePlacement, reloadSprites);
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
        CloseProject();

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
    data->projectPath = incomePath;
    DAVA::FilePath::AddTopResourcesFolder(data->GetDataPath());

    DAVA::TArc::PropertiesItem propsItem = GetAccessor().CreatePropertiesNode(ProjectManagerDetails::PROPERTIES_KEY);

    propsItem.Set(Settings::Internal_LastProjectPath.c_str(), DAVA::Any(data->projectPath));
    LoadMaterialsSettings(data);

    DAVA::QualitySettingsSystem::Instance()->Load("~res:/quality.yaml");
    DAVA::EngineContext* engineCtx = GetAccessor().GetEngineContext();
    engineCtx->soundSystem->InitFromQualitySettings();

    DAVA::FileSystem* fileSystem = engineCtx->fileSystem;
    fileSystem->CreateDirectory(data->GetWorkspacePath(), true);
    if (fileSystem->Exists(data->GetDataSourcePath()))
    {
        data->dataSourceSceneFiles->SetProjectDirectory(data->GetDataSourcePath());
    }

    // TODO remove imperative code that sync state of action
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
    }

    if (!projectPath.IsEmpty())
    {
        DVASSERT(projectPath.IsDirectoryPathname());
        delayedExecutor.DelayedExecute(DAVA::Bind(&ProjectManagerModule::OpenProjectByPath, this, projectPath));
    }
}

void ProjectManagerModule::CloseProject()
{
    ProjectManagerData* data = GetData();

    if (!data->projectPath.IsEmpty())
    {
        DVASSERT(data->closeProjectPredicate != nullptr);
        if (data->closeProjectPredicate())
        {
            // TODO remove imperative code that sync state of action
            if (closeAction)
            {
                closeAction->setEnabled(false);
            }
            DAVA::FilePath::RemoveResourcesFolder(data->GetDataPath());
            data->projectPath = "";

            SettingsManager::ResetPerProjectSettings();
            DAVA::TArc::PropertiesItem propsItem = GetAccessor().CreatePropertiesNode(ProjectManagerDetails::PROPERTIES_KEY);
            propsItem.Set(Settings::Internal_LastProjectPath.c_str(), DAVA::Any());
        }
    }
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
    if (GetAccessor().GetEngineContext()->fileSystem->Exists(materialsListPath))
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
                        if (DAVA::FileSystem::Instance()->Exists(templatePath))
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
