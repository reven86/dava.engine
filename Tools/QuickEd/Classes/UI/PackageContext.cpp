#include "PackageContext.h"

#include "Document.h"
#include "Package/PackageModel.h"
#include "Package/FilteredPackageModel.h"

#include <QItemSelection>

using namespace DAVA;

PackageContext::PackageContext(Document *_document)
    : QObject(_document)
    , document(_document)
    , model(_document->GetPackage(), _document->GetCommandExecutor(), this)
    , proxyModel(this)
{
    proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel.setSourceModel(&model);
}

PackageContext::~PackageContext()
{
    document = nullptr;
}

Document *PackageContext::GetDocument() const
{
    return document;
}

PackageModel *PackageContext::GetModel()
{
    return &model;
}

QSortFilterProxyModel *PackageContext::GetFilterProxyModel()
{
    return &proxyModel;
}

const QItemSelection &PackageContext::GetCurrentItemSelection() const
{
    return currentItemSelection;
}

void PackageContext::SetCurrentItemSelection(const QItemSelection &_currentItemSelection)
{
    currentItemSelection = _currentItemSelection;
}


const QString &PackageContext::GetFilterString() const
{
    return filterString;
}

const QList<QPersistentModelIndex> &PackageContext::GetExpandedIndexes() const
{
    return expandedIndexes;
}

void PackageContext::SetExpandedIndexes(const QList<QPersistentModelIndex> &_expandedIndexes)
{
    
    expandedIndexes = _expandedIndexes;
    return;
}