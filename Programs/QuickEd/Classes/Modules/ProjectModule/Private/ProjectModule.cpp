#include "Modules/ProjectModule/ProjectModule.h"
#include "Modules/ProjectModule/ProjectData.h"

#include "Application/QEGlobal.h"
#include "UI/mainwindow.h"

#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Engine/PlatformApi.h>
#include <FileSystem/FileSystem.h>
#include <PluginManager/PluginManager.h>
#include <Base/Result.h>

#include <QApplication>

DAVA_VIRTUAL_REFLECTION_IMPL(ProjectModule)
{
    DAVA::ReflectionRegistrator<ProjectModule>::Begin()
    .ConstructorByPointer()
    .End();
}

namespace ProjectModuleDetails
{
const DAVA::String propertiesKey = "ProjectModuleProperties";
const DAVA::String lastProjectKey = "Last project";
const DAVA::String projectsHistoryKey = "Projects history";
const DAVA::uint32 projectsHistoryMaxSize = 5;
}

ProjectModule::ProjectModule() = default;
ProjectModule::~ProjectModule() = default;

void ProjectModule::PostInit()
{
    CreateActions();
    RegisterOperation(QEGlobal::OpenLastProject.ID, this, &ProjectModule::OpenLastProject);
    RegisterOperation(ProjectModuleTesting::CreateProjectOperation.ID, this, &ProjectModule::CreateProject);

    delayedExecutor.DelayedExecute(MakeFunction(this, &ProjectModule::OpenLastProject));
}

void ProjectModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    CloseProject();
    DVASSERT(GetAccessor()->GetContextCount() == 0);
}

void ProjectModule::CreateActions()
{
    const QString toolBarName("mainToolbar");
    const QString fileMenuName("File");

    const QString newProjectActionName("New project");
    const QString openProjectActionName("Open project");
    const QString closeProjectActionName("Close project");
    const QString recentProjectsActionName("Recent");

    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();
    //action new project
    {
        QAction* action = new QAction(QIcon(":/Icons/newscene.png"), newProjectActionName, nullptr);
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&ProjectModule::OnNewProject, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuFile, { InsertionParams::eInsertionMethod::BeforeItem }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::BeforeItem }));

        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
    }

    //action open project
    {
        QAction* action = new QAction(QIcon(":/Icons/openproject.png"), openProjectActionName, nullptr);
        action->setShortcut(QKeySequence("Ctrl+O"));
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&ProjectModule::OnOpenProject, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuFile, { InsertionParams::eInsertionMethod::AfterItem, newProjectActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, newProjectActionName }));

        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
    }

    //action close project
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtTools/Icons/close-16.png"), closeProjectActionName);

        FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = DAVA::FastName(ProjectData::projectPathPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& fieldValue) -> DAVA::Any {
            return !fieldValue.Cast<DAVA::FilePath>(DAVA::FilePath()).IsEmpty();
        });

        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&ProjectModule::CloseProject, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuFile, { InsertionParams::eInsertionMethod::AfterItem, openProjectActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, openProjectActionName }));

        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
    }

    // RecentProjects
    {
        QAction* recentProjects = new QAction(recentProjectsActionName, nullptr);
        DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint(MenuItems::menuFile, DAVA::TArc::InsertionParams(InsertionParams::eInsertionMethod::AfterItem, closeProjectActionName)));
        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, recentProjects);
    }

    // Separator
    {
        QAction* separator = new QAction(nullptr);
        separator->setObjectName("project actions separator");
        separator->setSeparator(true);
        DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint("File", DAVA::TArc::InsertionParams(InsertionParams::eInsertionMethod::AfterItem, recentProjectsActionName)));
        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, separator);
    }

    //Recent content
    {
        RecentMenuItems::Params params(DAVA::TArc::mainWindowKey, accessor, ProjectModuleDetails::projectsHistoryKey);
        params.ui = GetUI();
        params.getMaximumCount = [this]() {
            return ProjectModuleDetails::projectsHistoryMaxSize;
        };
        params.menuSubPath << MenuItems::menuFile << recentProjectsActionName;
        params.insertionParams.method = InsertionParams::eInsertionMethod::BeforeItem;
        recentProjects.reset(new RecentMenuItems(std::move(params)));
        recentProjects->actionTriggered.Connect([this](const DAVA::String& projectPath) {
            OpenProject(projectPath);
        });
    }
}

void ProjectModule::OnOpenProject()
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();
    QString defaultPath;
    if (projectData != nullptr)
    {
        DAVA::String absProjectPath = projectData->GetProjectFile().GetAbsolutePathname();
        defaultPath = QString::fromStdString(absProjectPath);
    }

    FileDialogParams params;
    params.dir = defaultPath;
    params.filters = QObject::tr("Project files(*.quicked *.uieditor)");
    params.title = QObject::tr("Select a project file");
    QString projectPath = GetUI()->GetOpenFileName(DAVA::TArc::mainWindowKey, params);

    if (projectPath.isEmpty())
    {
        return;
    }
    projectPath = QDir::toNativeSeparators(projectPath);

    OpenProject(projectPath.toStdString());
}

void ProjectModule::OnNewProject()
{
    using namespace DAVA;
    using namespace TArc;

    DirectoryDialogParams params;
    params.title = QObject::tr("Select directory for new project");
    QString projectDirPath = GetUI()->GetExistingDirectory(DAVA::TArc::mainWindowKey, params);
    if (projectDirPath.isEmpty())
    {
        return;
    }

    CreateProject(projectDirPath);
}

void ProjectModule::CreateProject(const QString& projectDirPath)
{
    using namespace DAVA;
    using namespace TArc;

    ResultList resultList;

    QDir projectDir(projectDirPath);
    const QString projectFileName = QString::fromStdString(ProjectData::GetProjectFileName());
    QString fullProjectFilePath = projectDir.absoluteFilePath(projectFileName);
    if (QFile::exists(fullProjectFilePath))
    {
        ResultList resultList(Result(Result::RESULT_WARNING, QObject::tr("Project file exists!").toStdString()));
        ShowResultList(QObject::tr("Error while creating project"), resultList);
        return;
    }

    Result result = ProjectData::CreateNewProjectInfrastructure(fullProjectFilePath);
    if (result.type != Result::RESULT_SUCCESS)
    {
        ResultList resultList(result);
        ShowResultList(QObject::tr("Error while creating project"), resultList);
        return;
    }
    DVASSERT(fullProjectFilePath.isEmpty() == false);
    OpenProject(fullProjectFilePath.toStdString());
}

void ProjectModule::OpenProject(const DAVA::String& path)
{
    using namespace DAVA;
    using namespace TArc;

    if (CloseProject() == false)
    {
        return;
    }
    ContextAccessor* accessor = GetAccessor();

    ResultList resultList;
    std::unique_ptr<ProjectData> newProjectData = std::make_unique<ProjectData>();

    resultList = newProjectData->LoadProject(QString::fromStdString(path));

    if (resultList)
    {
        String lastProjectPath = newProjectData->GetProjectFile().GetAbsolutePathname();
        recentProjects->Add(lastProjectPath);

        PropertiesItem propsItem = accessor->CreatePropertiesNode(ProjectModuleDetails::propertiesKey);
        propsItem.Set(ProjectModuleDetails::lastProjectKey.c_str(), Any(lastProjectPath));

        DataContext* globalContext = accessor->GetGlobalContext();
        globalContext->CreateData(std::move(newProjectData));

        RegisterFolders();
        LoadPlugins();
    }
    ShowResultList(QObject::tr("Error while loading project"), resultList);
}

bool ProjectModule::CloseProject()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    InvokeOperation(QEGlobal::CloseAllDocuments.ID);
    //project was not closed
    if (accessor->GetContextCount() != 0)
    {
        return false;
    }
    if (globalContext->GetData<ProjectData>() != nullptr)
    {
        UnregisterFolders();
        globalContext->DeleteData<ProjectData>();
    }
    return true;
}

void ProjectModule::OpenLastProject()
{
    using namespace DAVA;
    using namespace TArc;
    ContextAccessor* accessor = GetAccessor();
    String projectPath;
    {
        PropertiesItem propsItem = accessor->CreatePropertiesNode(ProjectModuleDetails::propertiesKey);
        projectPath = propsItem.Get<DAVA::String>(ProjectModuleDetails::lastProjectKey);
    }
    if (projectPath.empty() == false)
    {
        OpenProject(projectPath);
    }
}

//TODO: move this function to the separate module when bubble messages will be implemented
void ProjectModule::LoadPlugins()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    const DataContext* globalContext = accessor->GetGlobalContext();
    const ProjectData* projectData = globalContext->GetData<ProjectData>();
    DVASSERT(projectData != nullptr);

    FilePath pluginsDirectory = projectData->GetPluginsDirectory().absolute;
    const EngineContext* engineContext = GetAccessor()->GetEngineContext();
    if (engineContext->fileSystem->IsDirectory(pluginsDirectory))
    {
        ResultList results;
        PluginManager* pluginManager = engineContext->pluginManager;
        Vector<FilePath> loadedPlugins = pluginManager->GetPlugins(pluginsDirectory, PluginManager::Auto);

        for (const FilePath& pluginPath : loadedPlugins)
        {
            const PluginDescriptor* descriptor = pluginManager->LoadPlugin(pluginPath);
            if (descriptor == nullptr)
            {
                results.AddResult(Result::RESULT_WARNING, Format("can not load plugin %s", pluginPath.GetAbsolutePathname().c_str()));
            }
        }
        ShowResultList(QObject::tr("Loading plugins"), results);
    }
}

void ProjectModule::RegisterFolders()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    const DataContext* globalContext = accessor->GetGlobalContext();
    const ProjectData* projectData = globalContext->GetData<ProjectData>();
    DVASSERT(projectData != nullptr);

    const EngineContext* engineContext = accessor->GetEngineContext();
    DAVA::FileSystem* fileSystem = engineContext->fileSystem;
    if (fileSystem->IsDirectory(projectData->GetAdditionalResourceDirectory().absolute))
    {
        FilePath::AddResourcesFolder(projectData->GetAdditionalResourceDirectory().absolute);
    }

    FilePath::AddResourcesFolder(projectData->GetConvertedResourceDirectory().absolute);
    FilePath::AddResourcesFolder(projectData->GetResourceDirectory().absolute);
}

void ProjectModule::UnregisterFolders()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    const DataContext* globalContext = accessor->GetGlobalContext();
    const ProjectData* projectData = globalContext->GetData<ProjectData>();
    DVASSERT(projectData != nullptr);

    const EngineContext* engineContext = GetAccessor()->GetEngineContext();
    DAVA::FileSystem* fileSystem = engineContext->fileSystem;
    if (fileSystem->IsDirectory(projectData->GetAdditionalResourceDirectory().absolute))
    {
        FilePath::RemoveResourcesFolder(projectData->GetAdditionalResourceDirectory().absolute);
    }

    FilePath::RemoveResourcesFolder(projectData->GetConvertedResourceDirectory().absolute);
    FilePath::RemoveResourcesFolder(projectData->GetResourceDirectory().absolute);
}

void ProjectModule::ShowResultList(const QString& title, const DAVA::ResultList& resultList)
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    ModalMessageParams params;
    params.title = title;

    QStringList errors;
    for (const Result& result : resultList.GetResults())
    {
        if (result.type == Result::RESULT_ERROR)
        {
            errors << QString::fromStdString(result.message);
        }
    }
    if (errors.empty())
    {
        return;
    }
    params.message = errors.join('\n');
    params.buttons = ModalMessageParams::Ok;
    UI* ui = GetUI();
    ui->ShowModalMessage(DAVA::TArc::mainWindowKey, params);
}

namespace ProjectModuleTesting
{
IMPL_OPERATION_ID(CreateProjectOperation);
}

DECL_GUI_MODULE(ProjectModule);
