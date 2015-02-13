#ifndef __UI_EDITOR_FILTERED_PACKAGE_MODEL_H__
#define __UI_EDITOR_FILTERED_PACKAGE_MODEL_H__

#include "FilteredPackageModel.h"
#include <QSortFilterProxyModel>

class FilteredPackageModel: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    FilteredPackageModel(QObject *parent = NULL);
    virtual ~FilteredPackageModel();

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    
    bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // __UI_EDITOR_FILTERED_PACKAGE_MODEL_H__
