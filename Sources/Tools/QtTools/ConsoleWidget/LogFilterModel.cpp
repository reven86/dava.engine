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

const QString& LogFilterModel::GetFilterString() const
{
    return filterText;
}

void LogFilterModel::SetFilters(const QVariantList & _filters)
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
        emit filterStringChanged(_filter);
    }
}

bool LogFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex source = sourceModel()->index(source_row, 0, source_parent);

    bool isAcceptedByText = true;
    if (!filterText.isEmpty())
    {
        const QString text = source.data(Qt::DisplayRole).toString();
        isAcceptedByText = text.contains(filterText, Qt::CaseInsensitive);
    }

    bool wasSet = false;
    const int level = source.data(LogModel::LEVEL_ROLE).toInt(&wasSet);
    const bool isAcceptedByLevel = (wasSet && filters.contains(QVariant( level )));

    return isAcceptedByLevel && isAcceptedByText;
}