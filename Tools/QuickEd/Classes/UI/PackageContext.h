#ifndef __QUICKED_PACKAGE_CONTEXT_H__
#define __QUICKED_PACKAGE_CONTEXT_H__

#include <QtCore>
#include "Package/PackageModel.h"
#include "Package/FilteredPackageModel.h"
class Document;
class PackageModel;
class FilteredPackageModel;

struct PackageContext 
{
    PackageContext(Document* document);
    PackageModel model;
    FilteredPackageModel proxyModel;
    QString filterString;
    QItemSelection currentItemSelection;
    QList<QPersistentModelIndex> expandedIndexes;
};


#endif // __QUICKED_PACKAGE_CONTEXT_H__
