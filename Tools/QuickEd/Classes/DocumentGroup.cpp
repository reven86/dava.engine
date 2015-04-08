#include "DocumentGroup.h"
#include <QObject>
#include "Document.h"
#include <QUndoGroup>
#include <QDebug>

#include "Debug/DVAssert.h"

DocumentGroup::DocumentGroup(QObject *parent) 
    : QObject(parent)
    , active(nullptr)
    , undoGroup(new QUndoGroup(this))
{
}

DocumentGroup::~DocumentGroup()
{
}

void DocumentGroup::AddDocument(Document* document)
{
    DVASSERT(document);
    undoGroup->addStack(document->GetUndoStack());
    if (documentList.contains(document))
    {
        return;
    }
    documentList.append(document);
}

void DocumentGroup::RemoveDocument(Document* document)
{
    DVASSERT(document);
    undoGroup->removeStack(document->GetUndoStack());

    if (0 == documentList.removeAll(document))
    {
        return;
    }
    if (document == active)
    {
        SetActiveDocument(nullptr);
    }
}

QList<Document*> DocumentGroup::GetDocuments() const
{
    return documentList;
}

void DocumentGroup::SetActiveDocument(Document* document)
{
    if (active == document)
    {
        return;
    }
    if (nullptr != active) 
    {
        disconnect(active, &Document::SharedDataChanged, this, &DocumentGroup::SharedDataChanged);
    }
    
    active = document;

    if (nullptr == active)
    {
        emit DocumentChanged(nullptr);
        undoGroup->setActiveStack(nullptr);
    }
    else
    {
        emit DocumentChanged(active->GetContext());
        
        connect(active, &Document::SharedDataChanged, this, &DocumentGroup::SharedDataChanged);

        undoGroup->setActiveStack(active->GetUndoStack());
    }
    emit ActiveDocumentChanged(document);
}

Document *DocumentGroup::GetActiveDocument() const
{
    return active;
}

const QUndoGroup *DocumentGroup::GetUndoGroup() const
{
    return undoGroup;
}
