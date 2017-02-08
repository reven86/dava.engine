#include "Modules/ProjectModule/ProjectModule.h"
#include "Modules/ProjectModule/ProjectData.h"

#include "Application/QEGlobal.h"
#include "UI/mainwindow.h"

#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Engine/PlatformApi.h>
#include <FileSystem/YamlNode.h>
#include <FileSystem/YamlEmitter.h>
#include <FileSystem/YamlParser.h>
#include <Base/Result.h>

#include <QApplication>

namespace ProjectModuleDetails
{
const DAVA::String propertiesKey = "ProjectModuleProperties";
const DAVA::String lastProjectKey = "Last project";
const DAVA::String projectsHistoryKey = "Projects history";
const DAVA::uint32 projectsHistoryMaxSize = 5;

bool SerializeProjectDataToFile(const ProjectData* data);
std::tuple<std::unique_ptr<ProjectData>, DAVA::ResultList> LoadFromFile(const QString& projectFile);
std::tuple<std::unique_ptr<ProjectData>, DAVA::ResultList> CreateProject(const QString& path);
DAVA::Result CreateProjectStructure(const QString& projectFilePath);
std::tuple<QString, DAVA::ResultList> CreateNewProject();
}

ProjectModule::ProjectModule() = default;
ProjectModule::~ProjectModule() = default;

void ProjectModule::PostInit()
{
    CreateActions();
    RegisterOperation(QEGlobal::OpenLastProject.ID, this, &ProjectModule::OpenLastProject);
}

void ProjectModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    CloseProject();
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
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/newscene.png"), newProjectActionName);
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&ProjectModule::OnNewProject, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(fileMenuName, { InsertionParams::eInsertionMethod::BeforeItem }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::BeforeItem }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    //action open project
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/openproject.png"), openProjectActionName);
        action->setShortcut(QKeySequence("Ctrl+O"));
        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&ProjectModule::OnOpenProject, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(fileMenuName, { InsertionParams::eInsertionMethod::AfterItem, newProjectActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, newProjectActionName }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    //action close project
    {
        QtAction* action = new QtAction(accessor, QIcon(":/QtTools/Icons/close-16.png"), closeProjectActionName);

        FieldDescriptor fieldDescr;
        fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = DAVA::FastName(ProjectData::projectPathPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& fieldValue) -> DAVA::Any {
            return fieldValue.CanCast<DAVA::FilePath>() && !fieldValue.Cast<DAVA::FilePath>().IsEmpty();
        });

        connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&ProjectModule::CloseProject, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(fileMenuName, { InsertionParams::eInsertionMethod::AfterItem, openProjectActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName, { InsertionParams::eInsertionMethod::AfterItem, openProjectActionName }));

        ui->AddAction(QEGlobal::windowKey, placementInfo, action);
    }

    // RecentProjects
    {
        QAction* recentProjects = new QAction(recentProjectsActionName, nullptr);
        DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint(fileMenuName, DAVA::TArc::InsertionParams(InsertionParams::eInsertionMethod::AfterItem, closeProjectActionName)));
        ui->AddAction(QEGlobal::windowKey, placementInfo, recentProjects);
    }

    // Separator
    {
        QAction* separator = new QAction("project actions separator", nullptr);
        separator->setSeparator(true);
        DAVA::TArc::ActionPlacementInfo placementInfo(DAVA::TArc::CreateMenuPoint("File", DAVA::TArc::InsertionParams(InsertionParams::eInsertionMethod::AfterItem, recentProjectsActionName)));
        ui->AddAction(QEGlobal::windowKey, placementInfo, separator);
    }

    //Recent content
    {
        RecentMenuItems::Params params(QEGlobal::windowKey);
        params.accessor = accessor;
        params.ui = GetUI();
        params.getMaximumCount = [this]() {
            return ProjectModuleDetails::projectsHistoryMaxSize;
        };

        params.getRecentFiles = [this]() -> DAVA::Vector<DAVA::String> {
            PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(ProjectModuleDetails::propertiesKey);
            return propsItem.Get<DAVA::Vector<DAVA::String>>(ProjectModuleDetails::projectsHistoryKey);
        };
        params.updateRecentFiles = [this](const DAVA::Vector<DAVA::String>& history) {
            PropertiesItem propsItem = GetAccessor()->CreatePropertiesNode(ProjectModuleDetails::propertiesKey);
            return propsItem.Set(ProjectModuleDetails::projectsHistoryKey, DAVA::Any(history));
        };
        params.menuSubPath << fileMenuName << recentProjectsActionName;
        params.insertionParams.method = InsertionParams::eInsertionMethod::BeforeItem;
        recentProjects.reset(new RecentMenuItems(params));
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
    QString projectPath = GetUI()->GetOpenFileName(QEGlobal::windowKey, params);

    if (projectPath.isEmpty())
    {
        return;
    }
    projectPath = QDir::toNativeSeparators(projectPath);

    OpenProject(projectPath.toStdString());
}

void ProjectModule::OnNewProject()
{
    DAVA::ResultList resultList;
    QString newProjectPath;

    std::tie(newProjectPath, resultList) = ProjectModuleDetails::CreateNewProject();

    if (!newProjectPath.isEmpty())
    {
        OpenProject(newProjectPath.toStdString());
    }
    ShowResultList(QObject::tr("Error while creating project"), resultList);
}

void ProjectModule::OpenProject(const DAVA::String& path)
{
    using namespace DAVA::TArc;
    if (!CloseProject())
    {
        return;
    }

    DAVA::ResultList resultList;
    std::unique_ptr<ProjectData> newProjectData;

    std::tie(newProjectData, resultList) = ProjectModuleDetails::CreateProject(QString::fromStdString(path));
    if (newProjectData)
    {
        DAVA::String lastProjectPath = newProjectData->GetProjectFile().GetAbsolutePathname();
        recentProjects->Add(lastProjectPath);

        ContextAccessor* accessor = GetAccessor();
        PropertiesItem propsItem = accessor->CreatePropertiesNode(ProjectModuleDetails::propertiesKey);
        propsItem.Set(ProjectModuleDetails::lastProjectKey.c_str(), DAVA::Any(lastProjectPath));

        DataContext* globalContext = accessor->GetGlobalContext();
        globalContext->CreateData(std::move(newProjectData));
    }
    ShowResultList(QObject::tr("Error while loading project"), resultList);
}

bool ProjectModule::CloseProject()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    InvokeOperation(QEGlobal::CloseAllDocuments.ID);
    if (accessor->GetContextCount() != 0)
    {
        return false;
    }

    globalContext->DeleteData<ProjectData>();
    return true;
}

void ProjectModule::OpenLastProject()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DAVA::String projectPath;
    {
        PropertiesItem propsItem = accessor->CreatePropertiesNode(ProjectModuleDetails::propertiesKey);
        projectPath = propsItem.Get<DAVA::String>(ProjectModuleDetails::lastProjectKey);
    }
    if (projectPath.empty() == false)
    {
        OpenProject(projectPath);
    }
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
        if (result.type == Result::RESULT_ERROR ||
            result.type == Result::RESULT_WARNING)
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
    delayedExecutor.DelayedExecute([ui, params]() {
        ui->ShowModalMessage(QEGlobal::windowKey, params);
    });
}

namespace ProjectModuleDetails
{
bool SerializeProjectDataToFile(const ProjectData* data)
{
    using namespace DAVA;

    RefPtr<YamlNode> node = ProjectData::SerializeToYamlNode(data);

    return YamlEmitter::SaveToYamlFile(data->GetProjectFile(), node.Get());
}

std::tuple<std::unique_ptr<ProjectData>, DAVA::ResultList> LoadFromFile(const QString& projectFile)
{
    using namespace DAVA;
    ResultList resultList;

    RefPtr<YamlParser> parser(YamlParser::Create(projectFile.toStdString()));
    if (parser.Get() == nullptr)
    {
        QString message = QObject::tr("Can not parse project file %1.").arg(projectFile);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());

        return std::make_tuple(std::unique_ptr<ProjectData>(), resultList);
    }

    return ProjectData::Parse(projectFile.toStdString(), parser->GetRootNode());
}

std::tuple<std::unique_ptr<ProjectData>, DAVA::ResultList> CreateProject(const QString& path)
{
    using namespace DAVA;
    ResultList resultList;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        QString message = QObject::tr("%1 does not exist.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return std::make_tuple(nullptr, resultList);
    }

    if (!fileInfo.isFile())
    {
        QString message = QObject::tr("%1 is not a file.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return std::make_tuple(nullptr, resultList);
    }

    return LoadFromFile(path);
}

DAVA::Result CreateProjectStructure(const QString& projectFilePath)
{
    using namespace DAVA;
    QDir projectDir(QFileInfo(projectFilePath).absolutePath());

    std::unique_ptr<ProjectData> defaultSettings = ProjectData::Default();

    QString resourceDirectory = QString::fromStdString(defaultSettings->GetResourceDirectory().relative);
    if (!projectDir.mkpath(resourceDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create resource directory %1.").arg(resourceDirectory).toStdString());
    }

    QString intermediateResourceDirectory = QString::fromStdString(defaultSettings->GetConvertedResourceDirectory().relative);
    if (intermediateResourceDirectory != resourceDirectory && !projectDir.mkpath(intermediateResourceDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create intermediate resource directory %1.").arg(intermediateResourceDirectory).toStdString());
    }

    QDir resourceDir(projectDir.absolutePath() + "/" + resourceDirectory);

    QString gfxDirectory = QString::fromStdString(defaultSettings->GetGfxDirectories().front().directory.relative);
    if (!resourceDir.mkpath(gfxDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create gfx directory %1.").arg(gfxDirectory).toStdString());
    }

    QString uiDirectory = QString::fromStdString(defaultSettings->GetUiDirectory().relative);
    if (!resourceDir.mkpath(uiDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create UI directory %1.").arg(uiDirectory).toStdString());
    }

    QString textsDirectory = QString::fromStdString(defaultSettings->GetTextsDirectory().relative);
    if (!resourceDir.mkpath(textsDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create UI directory %1.").arg(textsDirectory).toStdString());
    }

    QString fontsDirectory = QString::fromStdString(defaultSettings->GetFontsDirectory().relative);
    if (!resourceDir.mkpath(fontsDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create fonts directory %1.").arg(fontsDirectory).toStdString());
    }

    QString fontsConfigDirectory = QString::fromStdString(defaultSettings->GetFontsConfigsDirectory().relative);
    if (!resourceDir.mkpath(fontsConfigDirectory))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create fonts config directory %1.").arg(fontsConfigDirectory).toStdString());
    }

    QDir textsDir(resourceDir.absolutePath() + "/" + textsDirectory);
    QFile defaultLanguageTextFile(textsDir.absoluteFilePath(QString::fromStdString(defaultSettings->GetDefaultLanguage()) + ".yaml"));
    if (!defaultLanguageTextFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create localization file %1.").arg(defaultLanguageTextFile.fileName()).toStdString());
    }
    defaultLanguageTextFile.close();

    QDir fontsConfigDir(resourceDir.absolutePath() + "/" + fontsConfigDirectory);
    QFile defaultFontsConfigFile(fontsConfigDir.absoluteFilePath(QString::fromStdString(ProjectData::GetFontsConfigFileName())));
    if (!defaultFontsConfigFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create fonts config file %1.").arg(defaultFontsConfigFile.fileName()).toStdString());
    }
    defaultFontsConfigFile.close();

    defaultSettings->SetProjectFile(projectFilePath.toStdString());
    if (!ProjectModuleDetails::SerializeProjectDataToFile(defaultSettings.get()))
    {
        return Result(Result::RESULT_ERROR, QObject::tr("Can not create project file %1.").arg(projectFilePath).toStdString());
    }

    return Result();
}

std::tuple<QString, DAVA::ResultList> CreateNewProject()
{
    using namespace DAVA;

    ResultList resultList;

    QApplication* app = PlatformApi::Qt::GetApplication();

    QString projectDirPath = QFileDialog::getExistingDirectory(app->activeWindow(), QObject::tr("Select directory for new project"));
    if (projectDirPath.isEmpty())
    {
        return std::make_tuple(QString(), resultList);
    }

    QDir projectDir(projectDirPath);
    const QString projectFileName = QString::fromStdString(ProjectData::GetProjectFileName());
    QString fullProjectFilePath = projectDir.absoluteFilePath(projectFileName);
    if (QFile::exists(fullProjectFilePath))
    {
        resultList.AddResult(Result::RESULT_WARNING, QObject::tr("Project file exists!").toStdString());
        return std::make_tuple(QString(), resultList);
    }

    Result result = CreateProjectStructure(fullProjectFilePath);
    if (result.type != Result::RESULT_SUCCESS)
    {
        resultList.AddResult(result);
        return std::make_tuple(QString(), resultList);
    }

    return std::make_tuple(fullProjectFilePath, resultList);
}
}

DECL_GUI_MODULE(ProjectModule);
