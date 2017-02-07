#include "Modules/FileSystemCacheModule/FileSystemCacheModule.h"
#include "Modules/FileSystemCacheModule/FileSystemCacheData.h"
#include "Modules/ProjectModule/ProjectData.h"

#include "Application/QEGlobal.h"

#include "UI/mainwindow.h"
#include "UI/ProjectView.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>

#include <QtTools/ProjectInformation/FileSystemCache.h>
#include <QtTools/FileDialogs/FindFileDialog.h>

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>

DAVA_VIRTUAL_REFLECTION_IMPL(FileSystemCacheModule)
{
    DAVA::ReflectionRegistrator<FileSystemCacheModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FileSystemCacheModule::PostInit()
{
    using namespace DAVA;
    using namespace TArc;

    FieldDescriptor fieldDescr;
    fieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
    fieldDescr.fieldName = FastName(ProjectData::uiDirectoryPropertyName);
    projectUiPathFieldBinder = std::make_unique<FieldBinder>(GetAccessor());
    projectUiPathFieldBinder->BindField(fieldDescr, MakeFunction(this, &FileSystemCacheModule::OnUIPathChanged));

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<FileSystemCacheData>(QStringList() << "yaml"));

    CreateActions();
}

void FileSystemCacheModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    using namespace DAVA;
    using namespace TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->DeleteData<FileSystemCache>();
    projectUiPathFieldBinder.release();
}

void FileSystemCacheModule::OnUIPathChanged(const DAVA::Any& path)
{
    using namespace DAVA;
    using namespace TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();

    FileSystemCacheData* fileSystemCacheData = globalContext->GetData<FileSystemCacheData>();
    FileSystemCache* fileSystemCache = fileSystemCacheData->GetFileSystemCache();
    if (path.CanCast<ProjectData::ResDir>() == false)
    {
        fileSystemCache->UntrackAllDirectories();
        return;
    }
    ProjectData::ResDir uiDir = path.Cast<ProjectData::ResDir>();
    const FilePath& uiDirectory = uiDir.absolute;
    FileSystem* fileSystem = GetEngineContext()->fileSystem;
    DVASSERT(fileSystem->IsDirectory(uiDirectory));
    QString uiResourcesPath = QString::fromStdString(uiDirectory.GetStringValue());

    fileSystemCache->TrackDirectory(uiResourcesPath);
}

void FileSystemCacheModule::CreateActions()
{
    using namespace DAVA::TArc;
    const QString findFileInProjectActionName("Find file in project...");

    ContextAccessor* accessor = GetAccessor();

    QtAction* action = new QtAction(accessor, findFileInProjectActionName);
    action->setShortcutContext(Qt::ApplicationShortcut);
    action->setShortcuts(QList<QKeySequence>()
                         << Qt::CTRL + Qt::SHIFT + Qt::Key_O
                         << Qt::ALT + Qt::SHIFT + Qt::Key_O);
    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&FileSystemCacheModule::OnFindFile, this));
    FieldDescriptor fieldDescr;
    fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectData>();
    fieldDescr.fieldName = DAVA::FastName(ProjectData::uiDirectoryPropertyName);
    action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& fieldValue) -> DAVA::Any {
        return fieldValue.CanCast<ProjectData::ResDir>() && !fieldValue.Cast<ProjectData::ResDir>().absolute.IsEmpty();
    });

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint("Find", { InsertionParams::eInsertionMethod::BeforeItem }));

    GetUI()->AddAction(QEGlobal::windowKey, placementInfo, action);
}

void FileSystemCacheModule::OnFindFile()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    FileSystemCacheData* cacheData = globalContext->GetData<FileSystemCacheData>();
    FileSystemCache* cache = cacheData->GetFileSystemCache();
    MainWindow* mainWindow = globalContext->GetData<MainWindow>();
    DVASSERT(mainWindow != nullptr);
    DVASSERT(cache != nullptr);

    QString filePath = FindFileDialog::GetFilePath(cache, "yaml", mainWindow);
    if (filePath.isEmpty())
    {
        return;
    }
    mainWindow->GetProjectView()->SelectFile(filePath);
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, filePath);
}

DECL_GUI_MODULE(FileSystemCacheModule);
