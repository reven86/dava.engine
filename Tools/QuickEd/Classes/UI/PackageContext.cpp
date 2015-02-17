#include "PackageContext.h"

#include "Document.h"
#include "Package/PackageModel.h"
#include "Package/FilteredPackageModel.h"

#include <QItemSelection>

using namespace DAVA;

PackageContext::PackageContext(Document *_document)
    : QObject(_document), document(_document)
{
    model = new PackageModel(document);
    proxyModel = new FilteredPackageModel(this);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setSourceModel(model);
    
    currentSelection = new QItemSelection();

}

PackageContext::~PackageContext()
{
    proxyModel->setSourceModel(NULL);
    SafeDelete(proxyModel);
    
    SafeDelete(model);

    SafeDelete(currentSelection);

    document = nullptr;
}

Document *PackageContext::GetDocument() const
{
    return document;
}

PackageModel *PackageContext::GetModel() const
{
    return model;
}

QSortFilterProxyModel *PackageContext::GetFilterProxyModel() const
{
    return proxyModel;
}

const QString &PackageContext::GetFilterString() const
{
    return filterString;
}

QItemSelection *PackageContext::GetCurrentSelection() const
{
    return currentSelection;
}
