#pragma once

#include <QSortFilterProxyModel>

class PreferencesFilterModel : public QSortFilterProxyModel
{
public:
    PreferencesFilterModel(QObject* parent = nullptr);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};