#include "LibraryContext.h"

#include "Document.h"
#include "Library/LibraryModel.h"

LibraryContext::LibraryContext(Document *_document) : QObject(_document), document(_document)
{
    model = new LibraryModel(document->GetPackage(), this);
}

LibraryContext::~LibraryContext()
{
    delete model;
    model = nullptr;
}

Document *LibraryContext::GetDocument() const
{
    return document;
}

QAbstractItemModel *LibraryContext::GetModel() const
{
    return model;
}
