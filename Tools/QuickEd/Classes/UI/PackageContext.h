#ifndef __QUICKED_PACKAGE_CONTEXT_H__
#define __QUICKED_PACKAGE_CONTEXT_H__

#include <QObject>
#include <QPoint>
#include <QString>
#include <QPersistentModelIndex>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include "Package/PackageModel.h"
#include "Package/FilteredPackageModel.h"

class QSortFilterProxyModel;
class QItemSelection;
class PackageModel;
class Document;

class PackageContext : public QObject
{
    Q_OBJECT
    
public:
    PackageContext(Document *document);
    virtual ~PackageContext();
    
    Document *GetDocument() const;
    PackageModel *GetModel();
    QSortFilterProxyModel *GetFilterProxyModel();
    const QString &GetFilterString() const;

    const QItemSelection &GetCurrentItemSelection() const;
    void SetCurrentItemSelection(const QItemSelection &currentItemSelection);
    const QList<QPersistentModelIndex> &GetExpandedIndexes() const;
    void SetExpandedIndexes(const QList<QPersistentModelIndex> &expandedIndexes);
    
private:
    Document *document;

    QPoint scrollPosition;
    PackageModel model;
    FilteredPackageModel proxyModel;
    QString filterString;
    QItemSelection currentItemSelection;
    QList<QPersistentModelIndex> expandedIndexes;
};


#endif // __QUICKED_PACKAGE_CONTEXT_H__
