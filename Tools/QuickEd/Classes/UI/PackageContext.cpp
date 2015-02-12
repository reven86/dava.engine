#include "PackageContext.h"

#include "Document.h"
#include "PackageView/UIPackageModel.h"
#include "PackageView/UIFilteredPackageModel.h"

#include <QItemSelection>

using namespace DAVA;

PackageContext::PackageContext(Document *_document)
    : QObject(_document), document(_document)
{
    model = new UIPackageModel(document);
    proxyModel = new UIFilteredPackageModel(this);
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

UIPackageModel *PackageContext::GetModel() const
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
