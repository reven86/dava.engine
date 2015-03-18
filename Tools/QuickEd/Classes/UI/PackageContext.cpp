#include "PackageContext.h"

#include "Document.h"

PackageContext::PackageContext(Document *_document)
    : model(_document->GetPackage(), _document->GetCommandExecutor(), _document)
    , proxyModel(_document)
{
    proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel.setSourceModel(&model);
}