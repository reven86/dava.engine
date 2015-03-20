#ifndef QUICKED_DOCUMENTGROUP_H
#define QUICKED_DOCUMENTGROUP_H

#include <QObject>
#include <QUndoGroup>
class DocumentGroupPrivate;
class Document;
class ControlNode;
class QAbstractItemModel;
struct PackageContext;
class WidgetContext;

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

signals:
    void ActiveDocumentChanged(Document*);

    void LibraryDataChanged(const QString &role);
    void LibraryContextChanged(const WidgetContext *widgetContext);
    void PropertiesDataChanged(const QString &role);
    void PropertiesContextChanged(const WidgetContext *widgetContext);
    void PackageDataChanged(const QString &role);
    void PackageGontextChanged(const WidgetContext *widgetContext);
    void PreviewDataChanged(const QString &role);
    void PreviewContextChanged(const WidgetContext *widgetContext);

    void controlSelectedInEditor(ControlNode *activatedControls);
    void allControlsDeselectedInEditor();

    void OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls);
protected:
    QScopedPointer<DocumentGroupPrivate> d_ptr;
private:
    Q_DISABLE_COPY(DocumentGroup)
};

#endif // QUICKED_DOCUMENTGROUP_H
