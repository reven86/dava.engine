#include "LogFilterModel.h"

#include "LogModel.h"


LogFilterModel::LogFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    filters << DAVA::Logger::LEVEL_FRAMEWORK
        << DAVA::Logger::LEVEL_DEBUG 
        << DAVA::Logger::LEVEL_INFO
        << DAVA::Logger::LEVEL_WARNING
        << DAVA::Logger::LEVEL_ERROR;
}

LogFilterModel::~LogFilterModel()
{
}

const QVariantList &LogFilterModel::GetFilters() const
{
    return filters;
}

void LogFilterModel::SetFilters(const QVariantList & _filters)
{
    if (filters != _filters)
    {
        filters = _filters;
        invalidateFilter();
    }
}

bool LogFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex source = sourceModel()->index(source_row, 0, source_parent);
    bool wasSet = false;
    const int level = source.data(LogModel::LEVEL_ROLE).toInt(&wasSet);
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent) && wasSet && filters.contains(QVariant( level ));
}