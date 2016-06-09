#include "filterModel.h"
#include "listModel.h"

FilterModel::FilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

bool FilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    ListModel* model = dynamic_cast<ListModel*>(sourceModel());
    if (model != nullptr)
    {
        ListModel::ListItemType type = model->GetType(source_row);
        if (type == ListModel::LIST_ITEM_FAVORITES || type == ListModel::LIST_ITEM_SEPARATOR)
        {
            return true;
        }
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
