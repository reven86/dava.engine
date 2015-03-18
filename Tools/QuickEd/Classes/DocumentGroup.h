#ifndef QUICKED_DOCUMENTGROUP_H
#define QUICKED_DOCUMENTGROUP_H

#include <QObject>
#include <QUndoGroup>
class DocumentGroupPrivate;
class Document;
class ControlNode;
class QAbstractItemModel;
class PackageContext;

class DocumentGroup : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DocumentGroup)
public:
    explicit DocumentGroup(QObject *parent = 0);
    ~DocumentGroup();

    void AddDocument(Document*);
    void RemoveDocument(Document*);
    QList<Document*> GetDocuments() const;
    Document* GetActiveDocument() const;
    const QUndoGroup& GetUndoGroup() const;
public slots:
    void SetActiveDocument(Document* document);
    void OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);
signals:
    void ActiveDocumentChanged(Document*);
    void LibraryModelChanged(QAbstractItemModel *model);
    void PropertiesModelChanged(QAbstractItemModel *model);
    void PackageGontextChanged(PackageContext *context);


    void controlSelectedInEditor(ControlNode *activatedControls);
    void allControlsDeselectedInEditor();
protected:
    QScopedPointer<DocumentGroupPrivate> d_ptr;
private:
    Q_DISABLE_COPY(DocumentGroup)
};

#endif // QUICKED_DOCUMENTGROUP_H
