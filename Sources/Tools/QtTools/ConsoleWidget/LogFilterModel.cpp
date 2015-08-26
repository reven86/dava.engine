#include "LogFilterModel.h"

#include "LogModel.h"
#include "Debug/DVAssert.h"


LogFilterModel::LogFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

LogFilterModel::~LogFilterModel()
{
}

void LogFilterModel::SetFilters(const QVariantList & _filters)
{
    int tmpFilters = 0;
    for(const auto &filter : _filters)
    {
        DVASSERT(filter.canConvert<int>());
        tmpFilters |= (1 << filter.value<int>());
    }
    if(filters != tmpFilters)
    {
        filters = tmpFilters;
        invalidateFilter();
    }
}

bool LogFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex source = sourceModel()->index(source_row, 0, source_parent);
    bool wasSet = false;
    const int level = source.data(LogModel::LEVEL_ROLE).toInt(&wasSet);
    DVASSERT(wasSet);
    bool filterAccepted = wasSet && (filters & 1 << level );
    return  filterAccepted && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}