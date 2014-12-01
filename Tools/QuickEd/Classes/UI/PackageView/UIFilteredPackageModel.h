#ifndef __UI_EDITOR_FILTERED_PACKAGE_MODEL_H__
#define __UI_EDITOR_FILTERED_PACKAGE_MODEL_H__

#include "UIFilteredPackageModel.h"
#include <QSortFilterProxyModel>

class UIFilteredPackageModel: public QSortFilterProxyModel
{
    Q_OBJECT
public:
    UIFilteredPackageModel(QObject *parent = NULL);
    ~UIFilteredPackageModel();

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    
    bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // __UI_EDITOR_FILTERED_PACKAGE_MODEL_H__
