#include "Qt/DeviceInfo/MemoryTool/BacktraceSymbolTable.h"
#include "Qt/DeviceInfo/MemoryTool/Models/SymbolsListModel.h"

using namespace DAVA;

SymbolsListModel::SymbolsListModel(const BacktraceSymbolTable& symbolTable_, QObject* parent)
    : QAbstractListModel(parent)
    , symbolTable(symbolTable_)
{
    PrepareSymbols();
}

SymbolsListModel::~SymbolsListModel() = default;

int SymbolsListModel::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(allSymbols.size());
}

QVariant SymbolsListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        const String* name = allSymbols[index.row()];
        if (Qt::DisplayRole == role)
        {
            return QString(name->c_str());
        }
        else if (ROLE_SYMBOL_POINTER == role)
        {
            // Remove constness as QVariant does not accept const void*
            return QVariant::fromValue(const_cast<void*>(static_cast<const void*>(name)));
        }
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

//////////////////////////////////////////////////////////////////////////
SymbolsFilterModel::SymbolsFilterModel(SymbolsListModel* model_, QObject* parent)
    : QSortFilterProxyModel(parent)
{
    QSortFilterProxyModel::setSourceModel(model_);
}

void SymbolsFilterModel::SetFilterString(const QString& filterString)
{
    QString lowerCaseFilter = filterString.toLower();
    if (lowerCaseFilter != filter)
    {
        filter = lowerCaseFilter;
        invalidateFilter();
    }
}

void SymbolsFilterModel::ToggleHideStdAndUnresolved()
{
    hideStdAndUnresolved = !hideStdAndUnresolved;
    invalidateFilter();
}

bool SymbolsFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QAbstractItemModel* source = sourceModel();
    QString nameLeft = source->data(left, Qt::DisplayRole).toString();
    QString nameRight = source->data(right, Qt::DisplayRole).toString();
    return nameLeft < nameRight;
}

bool SymbolsFilterModel::filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const
{
    return true;
}

bool SymbolsFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QAbstractItemModel* source = sourceModel();
    QModelIndex index = source->index(source_row, 0, source_parent);

    QVariant v = source->data(index, SymbolsListModel::ROLE_SYMBOL_POINTER);
    const String* namePointer = static_cast<const String*>(v.value<void*>());
    Q_ASSERT(namePointer != nullptr);
    QString name = QString(namePointer->c_str()).toLower();

    bool decline = hideStdAndUnresolved && (name.startsWith("std::") || name.startsWith("#"));
    if (!decline)
    {
        decline = !name.contains(filter);
    }
    return !decline;
}
