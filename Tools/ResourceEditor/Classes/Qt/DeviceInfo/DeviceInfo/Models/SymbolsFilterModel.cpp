#include "Qt/DeviceInfo/DeviceInfo/Models/SymbolsFilterModel.h"
#include "Qt/DeviceInfo/DeviceInfo/Models/SymbolsListModel.h"

using namespace DAVA;

void SymbolsFilterModel::SetFilterString(const QString& filterString)
{
    QString lowerCaseFilter = filterString.toLower();
    if (lowerCaseFilter != filter)
    {
        filter = lowerCaseFilter;
        invalidateFilter();
    }
}

void SymbolsFilterModel::ToggleHideStdAndUnresolved()
{
    hideStdAndUnresolved = !hideStdAndUnresolved;
    invalidateFilter();
}

bool SymbolsFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QAbstractItemModel* source = sourceModel();
    QString nameLeft = source->data(left, Qt::DisplayRole).toString();
    QString nameRight = source->data(right, Qt::DisplayRole).toString();
    return nameLeft < nameRight;
}

bool SymbolsFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QAbstractItemModel* source = sourceModel();
    QModelIndex index = source->index(source_row, 0, source_parent);
    QString name = source->data(index, Qt::DisplayRole).toString().toLower();

    bool decline = hideStdAndUnresolved && (name.startsWith("std::") || name.startsWith("#"));
    if (!decline)
    {
        decline = !name.contains(filter);
    }
    return !decline;
}
