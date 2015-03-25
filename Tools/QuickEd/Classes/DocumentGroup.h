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

    void LibraryDataChanged(const QByteArray &role);
    void LibraryContextChanged(WidgetContext *widgetContext);
    void PropertiesDataChanged(const QByteArray &role);
    void PropertiesContextChanged(WidgetContext *widgetContext);
    void PackageDataChanged(const QByteArray &role);
    void PackageContextChanged(WidgetContext *widgetContext);
    void PreviewDataChanged(const QByteArray &role);
    void PreviewContextChanged(WidgetContext *widgetContext);
protected:
    QScopedPointer<DocumentGroupPrivate> d_ptr;
private:
    Q_DISABLE_COPY(DocumentGroup)
};

#endif // QUICKED_DOCUMENTGROUP_H
