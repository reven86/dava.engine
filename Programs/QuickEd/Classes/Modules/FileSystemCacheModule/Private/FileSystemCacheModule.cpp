#include "Modules/FileSystemCacheModule/FileSystemCacheModule.h"
#include "Modules/FileSystemCacheModule/FileSystemCacheData.h"
#include "Modules/ProjectModule/ProjectData.h"

#include "Application/QEGlobal.h"

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

    ContextAccessor* accessor = GetAccessor();
    projectDataWrapper = accessor->CreateWrapper(ReflectedTypeDB::Get<ProjectData>());
    projectDataWrapper.SetListener(this);

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
}

void FileSystemCacheModule::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA;
    using namespace TArc;

    if (fields.empty() == false &&
        std::find(fields.begin(), fields.end(), ProjectData::projectPathPropertyName) == fields.end())
    {
        return;
    }
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();

    FileSystemCacheData* fileSystemCacheData = globalContext->GetData<FileSystemCacheData>();
    FileSystemCache* fileSystemCache = fileSystemCacheData->GetFileSystemCache();

    fileSystemCache->UntrackAllDirectories();

    ProjectData* projectData = globalContext->GetData<ProjectData>();
    if (projectData == nullptr)
    {
        return;
    }
    FilePath uiDirectory = projectData->GetUiDirectory().absolute;
    FileSystem* fileSystem = GetEngineContext()->fileSystem;
    DVASSERT(fileSystem->IsDirectory(uiDirectory));
    QString uiResourcesPath = QString::fromStdString(uiDirectory.GetStringValue());

    fileSystemCache->TrackDirectory(uiResourcesPath);
}

void FileSystemCacheModule::CreateActions()
{
    using namespace DAVA;
    using namespace TArc;
    const QString findFileInProjectActionName("Find file in project...");

    ContextAccessor* accessor = GetAccessor();

    QtAction* action = new QtAction(accessor, findFileInProjectActionName);
    action->setShortcutContext(Qt::ApplicationShortcut);
    action->setShortcuts(QList<QKeySequence>()
                         << Qt::CTRL + Qt::SHIFT + Qt::Key_O
                         << Qt::ALT + Qt::SHIFT + Qt::Key_O);
    connections.AddConnection(action, &QAction::triggered, DAVA::Bind(&FileSystemCacheModule::FastOpenDocument, this));
    FieldDescriptor fieldDescr;
    fieldDescr.type = DAVA::ReflectedTypeDB::Get<ProjectData>();
    fieldDescr.fieldName = DAVA::FastName(ProjectData::projectPathPropertyName);
    action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const DAVA::Any& fieldValue) -> DAVA::Any {
        return !fieldValue.Cast<FilePath>(FilePath()).IsEmpty();
    });

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint("Find", { InsertionParams::eInsertionMethod::BeforeItem }));

    GetUI()->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
}

void FileSystemCacheModule::FastOpenDocument()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    FileSystemCacheData* cacheData = globalContext->GetData<FileSystemCacheData>();
    FileSystemCache* cache = cacheData->GetFileSystemCache();
    DVASSERT(cache != nullptr);

    QString filePath = FindFileDialog::GetFilePath(cache, "yaml", GetUI()->GetWindow(DAVA::TArc::mainWindowKey));
    if (filePath.isEmpty())
    {
        return;
    }
    InvokeOperation(QEGlobal::SelectFile.ID, filePath);
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, filePath);
}

DECL_GUI_MODULE(FileSystemCacheModule);
