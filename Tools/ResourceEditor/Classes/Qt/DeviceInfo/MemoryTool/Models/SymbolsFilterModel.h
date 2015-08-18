#pragma once

#include "Base/BaseTypes.h"

#include <QSortFilterProxyModel>

class SymbolsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SymbolsFilterModel() = default;
    virtual ~SymbolsFilterModel() = default;

public slots:
    void SetFilterString(const QString& filterString);
    void ToggleHideStdAndUnresolved();

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QString filter;
    bool hideStdAndUnresolved = true;   // If flag is set then filter out unresolved names and names beginning from std::
};
