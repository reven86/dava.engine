#pragma once

#include <QSortFilterProxyModel>

#include "Base/BaseTypes.h"

class SymbolsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SymbolsFilterModel();

public slots:
    void SetFilterString(const QString& filterString);
    void ToggleStd();

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    DAVA::String filter;
    bool hideStd;
};
