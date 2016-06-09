#pragma once

#include <QSortFilterProxyModel>

class FilterModel
: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FilterModel(QObject* parent = nullptr);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};
