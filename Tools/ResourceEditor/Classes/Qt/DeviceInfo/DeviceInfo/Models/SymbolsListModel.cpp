#include "Qt/DeviceInfo/DeviceInfo/BacktraceSymbolTable.h"
#include "Qt/DeviceInfo/DeviceInfo/Models/SymbolsListModel.h"

using namespace DAVA;

SymbolsListModel::SymbolsListModel(const BacktraceSymbolTable& symbolTable_, QObject* parent)
    : QAbstractListModel(parent)
    , symbolTable(symbolTable_)
{
    PrepareSymbols();
}

SymbolsListModel::~SymbolsListModel() = default;

const String* SymbolsListModel::Symbol(int row) const
{
    Q_ASSERT(row < static_cast<int>(allSymbols.size()));
    return allSymbols[row];
}

int SymbolsListModel::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(allSymbols.size());
}

QVariant SymbolsListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role)
    {
        const String* name = allSymbols[index.row()];
        return QString(name->c_str());
    }
    return QVariant();
}

void SymbolsListModel::PrepareSymbols()
{
    allSymbols.reserve(symbolTable.SymbolCount());
    symbolTable.IterateOverSymbols([this](const String& name) {
        allSymbols.push_back(&name);
    });
}
