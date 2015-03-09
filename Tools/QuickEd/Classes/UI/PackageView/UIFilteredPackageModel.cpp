#include "UIFilteredPackageModel.h"

#include "DAVAEngine.h"

UIFilteredPackageModel::UIFilteredPackageModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    
}

UIFilteredPackageModel::~UIFilteredPackageModel()
{
    
}

bool UIFilteredPackageModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
    {
        return true;
    }
    return hasAcceptedChildren(sourceRow, sourceParent);
}

QVariant UIFilteredPackageModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextColorRole)
    {
        QRegExp regExp = filterRegExp();
        if (!regExp.isEmpty())
        {
            QModelIndex srcIndex = mapToSource(index);
            if (!srcIndex.data(filterRole()).toString().contains(regExp))
                return Qt::lightGray;
        }
    }
    
    return QSortFilterProxyModel::data(index, role);
}

bool UIFilteredPackageModel::hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const
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
