#ifndef QUICKED_DOCUMENTGROUP_H
#define QUICKED_DOCUMENTGROUP_H

#include <QObject>
#include <QSet>
#include <QList>
#include <QPointer>

class Document;
class QUndoGroup;
class PackageBaseNode;
class QAction;
class QTabBar;

class DocumentGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSave READ CanSave NOTIFY CanSaveChanged)
    Q_PROPERTY(bool canClose READ CanClose NOTIFY CanCloseChanged)
public:
    explicit DocumentGroup(QObject* parent = nullptr);
    ~DocumentGroup();

    QList<Document*> GetDocuments() const;
    Document* GetActiveDocument() const;

    bool CanSave() const;
    bool CanClose() const;

    QAction* CreateUndoAction(QObject* parent, const QString& prefix = QString()) const;
    QAction* CreateRedoAction(QObject* parent, const QString& prefix = QString()) const;

    void AttachSaveAction(QAction* action) const;
    void AttachSaveAllAction(QAction* action) const;
    void AttachCloseDocumentAction(QAction* action) const;
    void AttachReloadDocumentAction(QAction* action) const;

    void ConnectToTabBar(QTabBar* tabBar);
    void DisconnectTabBar(QTabBar* tabBar);

signals:
    void ActiveIndexChanged(int index);
    void ActiveDocumentChanged(Document*);
    void CanSaveChanged(bool canSave);
    void CanSaveAllChanged(bool canSaveAll);
    void CanCloseChanged(bool canClose);

public slots:
    void AddDocument(const QString& path);

    bool TryCloseCurrentDocument();
    bool TryCloseDocument(int index);
    bool TryCloseDocument(Document* document);

    void CloseDocument(int index);
    void CloseDocument(Document* document);

    void ReloadDocument(int index, bool force);
    void ReloadDocument(Document* document, bool force);
    void ReloadCurrentDocument();

    void SetActiveDocument(int index);
    void SetActiveDocument(Document* document);
    void SaveAllDocuments();
    void SaveCurrentDocument();
    void OnCanSaveChanged(bool canSave);

private slots:
    void OnApplicationStateChanged(Qt::ApplicationState state);
    void OnFileChanged(Document* document);

private:
    void OnFilesChanged(const QList<Document*>& changedFiles);
    void OnFilesRemoved(const QList<Document*>& removedFiles);

    void ApplyFileChanges();
    int GetIndexByPackagePath(const QString& davaPath) const;
    void InsertTab(QTabBar* tabBar, Document* document, int index);
    void SaveDocument(Document* document, bool force);
    Document* CreateDocument(const QString& path);
    void InsertDocument(Document* document, int pos);
    DAVA::RefPtr<PackageNode> OpenPackage(const DAVA::FilePath& path);

    Document* active;
    QList<Document*> documents;
    QUndoGroup* undoGroup;
    QSet<Document*> changedFiles;
    QList<QPointer<QTabBar>> attachedTabBars;
};

#endif // QUICKED_DOCUMENTGROUP_H
