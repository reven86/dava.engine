#include "DocumentGroup.h"
#include <QObject>
#include "Document.h"
#include <QUndoGroup>
#include <QDebug>

class DocumentGroupPrivate
{
    Q_DECLARE_PUBLIC(DocumentGroup)
    DocumentGroup* const q_ptr;

public:
    DocumentGroupPrivate(DocumentGroup &object);
    Document *active;
    QList<Document*> documentList;
    QUndoGroup *undoGroup;
};

DocumentGroupPrivate::DocumentGroupPrivate(DocumentGroup &object)
    : q_ptr(&object)
    , active(nullptr)
    , undoGroup(new QUndoGroup(&object))
{
}

DocumentGroup::DocumentGroup(QObject *parent) 
    : QObject(parent)
    , d_ptr(new DocumentGroupPrivate(*this))
{
}

DocumentGroup::~DocumentGroup()
{
}

void DocumentGroup::AddDocument(Document* document)
{
    Q_D(DocumentGroup);
    Q_ASSERT(document);
    d->undoGroup->addStack(document->GetUndoStack());
    if (d->documentList.contains(document))
    {
        return;
    }
    d->documentList.append(document);
}

void DocumentGroup::RemoveDocument(Document* document)
{
    Q_D(DocumentGroup);
    Q_ASSERT(document);
    d->undoGroup->removeStack(document->GetUndoStack());

    if (0 == d->documentList.removeAll(document))
    {
        return;
    }
    if (document == d->active)
    {
        SetActiveDocument(nullptr);
    }
}

QList<Document*> DocumentGroup::GetDocuments() const
{
    Q_D(const DocumentGroup);
    return d->documentList;
}

void DocumentGroup::SetActiveDocument(Document* document)
{
    Q_D(DocumentGroup);
    if (d->active == document)
    {
        return;
    }
    if (nullptr != d->active) 
    {
        disconnect(d->active, &Document::ContextDataChanged, this, &DocumentGroup::ContextDataChanged);
    }
    
    d->active = document;

    if (nullptr == d->active)
    {
        emit DocumentChanged(nullptr);
        d->undoGroup->setActiveStack(nullptr);
    }
    else
    {
        emit DocumentChanged(d->active->GetContext());
        
        connect(d->active, &Document::ContextDataChanged, this, &DocumentGroup::ContextDataChanged);

        d->undoGroup->setActiveStack(d->active->GetUndoStack());
    }
    emit ActiveDocumentChanged(document);
}

Document *DocumentGroup::GetActiveDocument() const
{
    Q_D(const DocumentGroup);
    return d->active;
}

const QUndoGroup *DocumentGroup::GetUndoGroup() const
{
    Q_D(const DocumentGroup);
    return d->undoGroup;
}
