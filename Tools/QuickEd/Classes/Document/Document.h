#ifndef __QUICKED_DOCUMENT_H__
#define __QUICKED_DOCUMENT_H__

#include <QObject>
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "EditorSystems/SelectionContainer.h"

struct WidgetContext
{
    virtual ~WidgetContext() = 0;
};

inline WidgetContext::~WidgetContext()
{
}

namespace DAVA
{
class FilePath;
class UIControl;
class UIEvent;
class CommandStack;
}

class PackageNode;
class QtModelPackageCommandExecutor;

class PropertiesModel;
class PackageModel;
class ControlNode;
class AbstractProperty;

class QFileSystemWatcher;

class Document final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSave READ CanSave NOTIFY CanSaveChanged);
    Q_PROPERTY(bool canClose READ CanClose WRITE SetCanClose NOTIFY CanCloseChanged);

public:
    explicit Document(const DAVA::RefPtr<PackageNode>& package, QObject* parent = nullptr);
    ~Document();

    const DAVA::FilePath& GetPackageFilePath() const;
    QString GetPackageAbsolutePath() const;
    DAVA::CommandStack* GetCommandStack() const;
    PackageNode* GetPackage() const;
    QtModelPackageCommandExecutor* GetCommandExecutor() const;
    WidgetContext* GetContext(void* requester) const;
    void Save();

    void SetContext(void* requester, WidgetContext* widgetContext);
    void RefreshLayout();
    bool CanSave() const;
    bool CanClose() const;
    bool IsDocumentExists() const;

signals:
    void FileChanged(Document* document);
    void CanSaveChanged(bool canSave);
    void CanCloseChanged(bool canClose);

public slots:
    void RefreshAllControlProperties();
    void SetCanClose(bool canClose);

private slots:
    void OnFileChanged(const QString& path);
    void OnCleanChanged(bool clean);

private:
    void SetCanSave(bool canSave);
    DAVA::UnorderedMap<void*, WidgetContext*> contexts;

    DAVA::RefPtr<PackageNode> package;
    std::unique_ptr<QtModelPackageCommandExecutor> commandExecutor;
    std::unique_ptr<DAVA::CommandStack> commandStack;
    QFileSystemWatcher* fileSystemWatcher = nullptr;
    bool fileExists = true;
    bool canSave = false;
    bool canClose = true;
};

#endif // __QUICKED_DOCUMENT_H__
