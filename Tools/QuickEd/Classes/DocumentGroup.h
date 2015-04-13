#ifndef QUICKED_DOCUMENTGROUP_H
#define QUICKED_DOCUMENTGROUP_H

#include <QObject>
#include <QUndoGroup>

class Document;
class SharedData;

class DocumentGroup : public QObject
{
    Q_OBJECT
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

    void SharedDataChanged(const QByteArray &role);
    void DocumentChanged(SharedData *data);
protected:
    Document *active;
    QList<Document*> documentList;
    QUndoGroup *undoGroup;
};

#endif // QUICKED_DOCUMENTGROUP_H
