#include "FilteredPackageModel.h"
#include <QColor>
#include "DAVAEngine.h"

FilteredPackageModel::FilteredPackageModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    
}

FilteredPackageModel::~FilteredPackageModel()
{
    
}

bool FilteredPackageModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
    {
        return true;
    }
    return hasAcceptedChildren(sourceRow, sourceParent);
}

QVariant FilteredPackageModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextColorRole)
    {
        QRegExp regExp = filterRegExp();
        if (!regExp.isEmpty())
        {
            QModelIndex srcIndex = mapToSource(index);
            if (!srcIndex.data(filterRole()).toString().contains(regExp))
                return QColor(Qt::lightGray);
        }
    }
    
    return QSortFilterProxyModel::data(index, role);
}

bool FilteredPackageModel::hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex item = sourceModel()->index(sourceRow, 0, sourceParent);
    int rowCount = sourceModel()->rowCount(item);
    
    if (rowCount == 0)
        return false;
    
    for (int row = 0; row < rowCount; ++row)
    {
        if (filterAcceptsRow(row, item))
            return true;
    }
    
    return false;
}
