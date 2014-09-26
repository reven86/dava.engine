#ifndef __LOGFILTERMODEL_H__
#define __LOGFILTERMODEL_H__


#include <QObject>
#include <QSortFilterProxyModel>
#include <QSet>


class LogFilterModel
    : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit LogFilterModel(QObject* parent = NULL);
    ~LogFilterModel();

public slots:
    void SetFilters(const QSet<int>& filters);
    void SetFilterString(const QString& filter);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

    QSet<int> filters;
    QString filterText;
};


#endif // __LOGFILTERMODEL_H__
