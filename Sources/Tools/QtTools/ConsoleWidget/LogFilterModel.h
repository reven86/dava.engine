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
    
    const QVariantList &GetFilters() const;
public slots:
    void SetFilters(const QVariantList &filters);
signals:
    void filterStringChanged(const QString filter);
private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    QVariantList filters;
    QString filterText;
};
#endif // __LOGFILTERMODEL_H__
