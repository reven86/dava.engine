#pragma once

#include "Base/BaseTypes.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

class BacktraceSymbolTable;

class SymbolsListModel : public QAbstractListModel
{
public:
    enum
    {
        ROLE_SYMBOL_POINTER = Qt::UserRole + 1
    };

public:
    SymbolsListModel(const BacktraceSymbolTable& symbolTable, QObject* parent = nullptr);
    virtual ~SymbolsListModel();

    // QAbstractListModel
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    void PrepareSymbols();

private:
    const BacktraceSymbolTable& symbolTable;
    DAVA::Vector<const DAVA::String*> allSymbols;
};

//////////////////////////////////////////////////////////////////////////
class SymbolsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SymbolsFilterModel(SymbolsListModel* model, QObject* parent = nullptr);
    virtual ~SymbolsFilterModel() = default;

public slots:
    void SetFilterString(const QString& filterString);
    void ToggleHideStdAndUnresolved();

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QString filter;
    bool hideStdAndUnresolved = true;   // If flag is set then filter out unresolved names and names beginning from std::
};
