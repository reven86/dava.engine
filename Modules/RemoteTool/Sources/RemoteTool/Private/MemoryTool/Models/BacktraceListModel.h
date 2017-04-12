#pragma once

#include <Base/BaseTypes.h>

#include <QAbstractListModel>

class BacktraceSymbolTable;

class BacktraceListModel : public QAbstractListModel
{
public:
    enum
    {
        ROLE_SYMBOL_POINTER = Qt::UserRole + 1
    };

public:
    BacktraceListModel(const BacktraceSymbolTable& symbolTable, QObject* parent = nullptr);
    virtual ~BacktraceListModel();

    void Update(DAVA::uint32 backtraceHash);
    void Clear();

    // QAbstractListModel
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    const BacktraceSymbolTable& symbolTable;
    const DAVA::Vector<const DAVA::String*>* symbols = nullptr;
};
