#include "Document.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/YamlPackageSerializer.h"

#include "Ui/QtModelPackageCommandExecutor.h"
#include "EditorCore.h"

using namespace DAVA;
using namespace std;
using namespace placeholders;

Document::Document(const RefPtr<PackageNode>& package_, QObject* parent)
    : QObject(parent)
    , package(package_)
    , commandExecutor(new QtModelPackageCommandExecutor(this))
    , undoStack(new QUndoStack(this))
    , fileSystemWatcher(new QFileSystemWatcher(this))
{
    QString path = GetPackageAbsolutePath();
    DVASSERT(QFile::exists(path));
    if (!fileSystemWatcher->addPath(path))
    {
        DAVA::Logger::Error("can not add path to the file watcher: %s", path.toUtf8().data());
    }
    connect(GetEditorFontSystem(), &EditorFontSystem::UpdateFontPreset, this, &Document::RefreshAllControlProperties);
    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &Document::OnFileChanged, Qt::DirectConnection);
    connect(undoStack.get(), &QUndoStack::cleanChanged, this, &Document::OnCleanChanged);
}

Document::~Document()
{
    disconnect(undoStack.get(), &QUndoStack::cleanChanged, this, &Document::OnCleanChanged); //destructor of UndoStack send signal here
    for (auto& context : contexts)
    {
        delete context.second;
    }
}

const FilePath& Document::GetPackageFilePath() const
{
    return package->GetPath();
}

QString Document::GetPackageAbsolutePath() const
{
    return QString::fromStdString(GetPackageFilePath().GetAbsolutePathname());
}

QUndoStack* Document::GetUndoStack() const
{
    return undoStack.get();
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
    QString path = GetPackageAbsolutePath();
    fileSystemWatcher->removePath(path);
    YamlPackageSerializer serializer;
    serializer.SerializePackage(package.Get());
    serializer.WriteToFile(package->GetPath());
    undoStack->setClean();
    if (!fileSystemWatcher->addPath(path))
    {
        DAVA::Logger::Error("can not add path to the file watcher: %s", path.toUtf8().data());
    }
}

void Document::SetContext(void* requester, WidgetContext* widgetContext)
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        DVASSERT_MSG(false, "document already have this context");
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

void Document::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
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
    SetCanSave(!fileExists || !undoStack->isClean());
    emit FileChanged(this);
}

void Document::OnCleanChanged(bool clean)
{
    SetCanSave(fileExists && !clean);
}
