#ifndef QUICKED_DOCUMENTGROUP_H
#define QUICKED_DOCUMENTGROUP_H

#include <QObject>
#include <QUndoGroup>
class DocumentGroupPrivate;
class Document;
class WidgetContext;

class DocumentGroup : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DocumentGroup)
public:
    explicit DocumentGroup(QObject *parent = nullptr);
    ~DocumentGroup();

    void AddDocument(Document*);
    void RemoveDocument(Document*);
    QList<Document*> GetDocuments() const;
    Document* GetActiveDocument() const;
    const QUndoGroup* GetUndoGroup() const;
public slots:
    void SetActiveDocument(Document* document);

signals:
    void ActiveDocumentChanged(Document*);

    void ContextDataChanged(const QByteArray &role);
    void ContextChanged(WidgetContext *widgetContext);
protected:
    QScopedPointer<DocumentGroupPrivate> d_ptr;
};

#endif // QUICKED_DOCUMENTGROUP_H
