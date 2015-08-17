#include "Debug/DVAssert.h"

#include "Qt/DeviceInfo/DeviceInfo/BacktraceSymbolTable.h"
#include "Qt/DeviceInfo/DeviceInfo/Models/SymbolsTreeModel.h"

using namespace DAVA;

SymbolsTreeModel::SymbolsTreeModel(const BacktraceSymbolTable& symbolTable_, QObject* parent)
    : QAbstractListModel(parent)
    , symbolTable(symbolTable_)
{
    PrepareSymbols();
}

SymbolsTreeModel::~SymbolsTreeModel() = default;

const String* SymbolsTreeModel::Symbol(int row) const
{
    DVASSERT(row < static_cast<int>(allSymbols.size()));
    return allSymbols[row];
}

int SymbolsTreeModel::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(allSymbols.size());
}

QVariant SymbolsTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role)
    {
        const String* name = allSymbols[index.row()];
        return QString(name->c_str());
    }
    return QVariant();
}

void SymbolsTreeModel::PrepareSymbols()
{
    allSymbols.reserve(symbolTable.SymbolCount());
    symbolTable.IterateOverSymbols([this](const String& name) {
        allSymbols.push_back(&name);
    });
}
