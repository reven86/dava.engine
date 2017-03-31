#include "RemoteTool/Private/MemoryTool/Models/BacktraceListModel.h"
#include "RemoteTool/Private/MemoryTool/BacktraceSymbolTable.h"

using namespace DAVA;

BacktraceListModel::BacktraceListModel(const BacktraceSymbolTable& symbolTable_, QObject* parent)
    : QAbstractListModel(parent)
    , symbolTable(symbolTable_)
{
}

BacktraceListModel::~BacktraceListModel() = default;

void BacktraceListModel::Update(DAVA::uint32 backtraceHash)
{
    beginResetModel();
    symbols = symbolTable.GetBacktraceSymbols(backtraceHash);
    endResetModel();
}

void BacktraceListModel::Clear()
{
    beginResetModel();
    symbols = nullptr;
    endResetModel();
}

int BacktraceListModel::rowCount(const QModelIndex& parent) const
{
    return symbols != nullptr ? static_cast<int>(symbols->size()) : 0;
}

QVariant BacktraceListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        const String* name = (*symbols)[index.row()];
        if (Qt::DisplayRole == role)
        {
            return QString(index.row(), QChar(' ')) + QString(name->c_str());
        }
        else if (ROLE_SYMBOL_POINTER == role)
        {
            // Remove constness as QVariant does not accept const void*
            return QVariant::fromValue(const_cast<void*>(static_cast<const void*>(name)));
        }
    }
    return QVariant();
}
