#include "DocumentGroup.h"
#include <QObject>
#include "Model/PackageHierarchy/ControlNode.h"
#include "Document.h"
#include <QUndoGroup>
#include <QAbstractItemModel>

class DocumentGroupPrivate
{
    Q_DECLARE_PUBLIC(DocumentGroup)
    DocumentGroup* const q_ptr;

public:
    DocumentGroupPrivate(DocumentGroup &object);
    Document *active;
    QList<Document*> documentList;
    QUndoGroup undoGroup;
};

DocumentGroupPrivate::DocumentGroupPrivate(DocumentGroup &object)
    : q_ptr(&object)
    , active(nullptr)
    , undoGroup(&object)
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
    d->undoGroup.addStack(document->GetUndoStack());
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
    d->undoGroup.removeStack(document->GetUndoStack());

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
        disconnect(d->active, &Document::controlSelectedInEditor, this, &DocumentGroup::controlSelectedInEditor);
        disconnect(d->active, &Document::allControlsDeselectedInEditor, this, &DocumentGroup::allControlsDeselectedInEditor);
    }
    
    d->active = document;

    if (nullptr == d->active)
    {
        emit LibraryModelChanged(nullptr);
        emit PropertiesModelChanged(nullptr);
        //!!check that is actual
        emit controlSelectedInEditor(nullptr);
        emit allControlsDeselectedInEditor();
        d->undoGroup.setActiveStack(nullptr);
    }
    else
    {
        emit LibraryModelChanged(d->active->GetLibraryModel());
        emit PropertiesModelChanged(reinterpret_cast<QAbstractItemModel*>(d->active->GetPropertiesModel()));
        //!!emit PropertiesModelChanged(static_cast<QAbstractItemModel*>(d->active->GetPropertiesModel()));

        connect(d->active, &Document::controlSelectedInEditor, this, &DocumentGroup::controlSelectedInEditor);
        connect(d->active, &Document::allControlsDeselectedInEditor, this, &DocumentGroup::allControlsDeselectedInEditor);
        d->undoGroup.setActiveStack(d->active->GetUndoStack());
    }
    emit ActiveDocumentChanged(document);
}

void DocumentGroup::OnSelectionRootControlChanged(const QList<ControlNode *> &activatedRootControls, const QList<ControlNode *> &deactivatedRootControls)
{
    Q_D(DocumentGroup);
    if (nullptr != d->active)
    {
        d->active->OnSelectionRootControlChanged(activatedRootControls, deactivatedRootControls);
    }
}

void DocumentGroup::OnSelectionControlChanged(const QList<ControlNode *> &activatedControls, const QList<ControlNode *> &deactivatedControls)
{
    Q_D(DocumentGroup);
    if (nullptr != d->active)
    {
        d->active->OnSelectionControlChanged(activatedControls, deactivatedControls);
    }
}

Document *DocumentGroup::GetActiveDocument() const
{
    Q_D(const DocumentGroup);
    return d->active;
}

const QUndoGroup& DocumentGroup::GetUndoGroup() const
{
    Q_D(const DocumentGroup);
    return d->undoGroup;
}
