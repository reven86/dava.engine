#pragma once

#include "Base/BaseTypes.h"

#include <QAbstractListModel>

class BacktraceSymbolTable;

class SymbolsListModel : public QAbstractListModel
{
public:
    SymbolsListModel(const BacktraceSymbolTable& symbolTable, QObject* parent = nullptr);
    virtual ~SymbolsListModel();

    const DAVA::String* Symbol(int row) const;

    // QAbstractListModel
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    void PrepareSymbols();

private:
    const BacktraceSymbolTable& symbolTable;
    DAVA::Vector<const DAVA::String*> allSymbols;
};
