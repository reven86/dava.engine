#ifndef __LOGFILTERMODEL_H__
#define __LOGFILTERMODEL_H__


#include <QObject>
#include <QSortFilterProxyModel>
#include <QFlags>
#include "FileSystem/Logger.h"


class LogFilterModel
    : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    Q_DECLARE_FLAGS(LogLevels, DAVA::Logger::eLogLevel);

    explicit LogFilterModel(QObject* parent = nullptr);
    ~LogFilterModel();
    
    const LogLevels &GetFilters() const;
    const QString &GetFilterString() const;
public slots:
    void SetFilters(const LogLevels &filters);
    void SetFilterString(const QString& filter);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    LogLevels filters;
    QString filterText;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LogFilterModel::LogLevels);
#endif // __LOGFILTERMODEL_H__
