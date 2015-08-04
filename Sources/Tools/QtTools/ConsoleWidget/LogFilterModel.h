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
    explicit LogFilterModel(QObject* parent = nullptr);
    ~LogFilterModel();
public slots:
    void SetFilters(const QVariantList &filters);
private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    quint64 filters = ~0;
};
#endif // __LOGFILTERMODEL_H__
