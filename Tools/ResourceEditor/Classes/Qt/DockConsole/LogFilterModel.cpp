#include "LogFilterModel.h"

#include <QDebug>

#include "LogModel.h"


LogFilterModel::LogFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

LogFilterModel::~LogFilterModel()
{
}

void LogFilterModel::SetFilters(const QSet<int>& _filters)
{
    if (filters != _filters)
    {
        filters = _filters;
        invalidateFilter();
    }
}

void LogFilterModel::SetFilterString(const QString& _filter)
{
    if (_filter != filterText)
    {
        filterText = _filter;
        invalidateFilter();
    }
}

bool LogFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex source = sourceModel()->index(source_row, 0, source_parent);

    bool isAcceptedByText = true;
    if (!filterText.isEmpty())
    {
        const QString text = source.data(Qt::DisplayRole).toString();
        if (!text.contains(filterText, Qt::CaseInsensitive))
        {
            isAcceptedByText = false;
        }
    }

    bool wasSet = false;
    const int level = source.data(LogModel::LEVEL_ROLE).toInt(&wasSet);
    const bool isAcceptedByLevel = (wasSet && filters.contains(level));

    return isAcceptedByLevel && isAcceptedByText;
}