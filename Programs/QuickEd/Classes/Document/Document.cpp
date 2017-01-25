#include "Document/Document.h"
#include "Command/CommandStack.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/YamlPackageSerializer.h"
#include "Project/Project.h"

#include "Logger/Logger.h"

#include "Ui/QtModelPackageCommandExecutor.h"
#include <QFileSystemWatcher>
#include <QFile>
#include <QFileInfo>
#include <QSignalBlocker>

using namespace DAVA;
using namespace std;
using namespace placeholders;

Document::Document(Project* project_, const RefPtr<PackageNode>& package_, QObject* parent)
    : QObject(parent)
    , package(package_)
    , commandExecutor(new QtModelPackageCommandExecutor(project_, this))
    , commandStack(new CommandStack())
    , project(project_)
{
    CreateFileSystemWatcher();
    commandStack->cleanChanged.Connect(this, &Document::OnCleanChanged);
}

Document::~Document()
{
    for (auto& context : contexts)
    {
        delete context.second;
    }
    DVASSERT(CanClose());
}

const FilePath& Document::GetPackageFilePath() const
{
    return package->GetPath();
}

QString Document::GetPackageAbsolutePath() const
{
    return QString::fromStdString(GetPackageFilePath().GetAbsolutePathname());
}

CommandStack* Document::GetCommandStack() const
{
    return commandStack.get();
}

PackageNode* Document::GetPackage() const
{
    return package.Get();
}

QtModelPackageCommandExecutor* Document::GetCommandExecutor() const
{
    return commandExecutor.get();
}

WidgetContext* Document::GetContext(void* requester) const
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        return iter->second;
    }
    return nullptr;
}

void Document::Save()
{
    SafeDelete(fileSystemWatcher);
    YamlPackageSerializer serializer;
    serializer.SerializePackage(package.Get());
    serializer.WriteToFile(package->GetPath());
    commandStack->SetClean();
    CreateFileSystemWatcher();
}

void Document::SetContext(void* requester, WidgetContext* widgetContext)
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        DVASSERT(false, "document already have this context");
        delete iter->second;
        contexts.erase(iter);
    }
    contexts.emplace(requester, widgetContext);
}

void Document::RefreshLayout()
{
    package->RefreshPackageStylesAndLayout(true);
}

bool Document::CanSave() const
{
    return canSave;
}

bool Document::IsDocumentExists() const
{
    return fileExists;
}

void Document::OnFontPresetChanged(const DAVA::String& presetName)
{
    RefreshAllControlProperties();
}

void Document::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
}

Project* Document::GetProject() const
{
    return project;
}

void Document::SetCanSave(bool arg)
{
    if (arg != canSave)
    {
        canSave = arg;
        CanSaveChanged(arg);
    }
}

void Document::OnFileChanged(const QString& path)
{
    DVASSERT(path == GetPackageAbsolutePath());
    fileExists = QFile::exists(GetPackageAbsolutePath());
    SetCanSave(!fileExists || !commandStack->IsClean());
    emit FileChanged(this);
}

void Document::OnCleanChanged(bool clean)
{
    SetCanSave(fileExists && !clean);
}

bool Document::CanClose() const
{
    return canClose;
}

void Document::SetCanClose(bool canClose_)
{
    if (canClose != canClose_)
    {
        canClose = canClose_;
        emit CanCloseChanged(canClose);
    }
}

QString Document::GetName() const
{
    QFileInfo fileInfo(GetPackageAbsolutePath());
    return fileInfo.fileName();
}

void Document::CreateFileSystemWatcher()
{
    DVASSERT(fileSystemWatcher == nullptr);
    fileSystemWatcher = new QFileSystemWatcher(this);
    QString path = GetPackageAbsolutePath();
    if (!fileSystemWatcher->addPath(path))
    {
        DAVA::Logger::Error("can not add path to the file watcher: %s", path.toUtf8().data());
    }
    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &Document::OnFileChanged, Qt::DirectConnection);
}
