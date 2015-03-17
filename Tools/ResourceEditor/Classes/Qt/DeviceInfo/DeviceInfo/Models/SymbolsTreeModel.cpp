#include <QDebug>
#include "SymbolsTreeModel.h"

using namespace DAVA;

SymbolsTreeModel::SymbolsTreeModel(const std::unordered_map<DAVA::uint64, DAVA::String>& symbols,
                                   QObject* parent)
    : QAbstractItemModel(parent)
    , symbolMap(symbols)
    , rootNode(new GenericTreeNode)
{
    BuildTree();
}

SymbolsTreeModel::~SymbolsTreeModel()
{

}

QVariant SymbolsTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role)
    {
        GenericTreeNode* node = static_cast<GenericTreeNode*>(index.internalPointer());
        return node->Data(index.row(), index.column());
    }
    return QVariant();
}

QVariant SymbolsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Horizontal == orientation && Qt::DisplayRole == role && 0 <= section && section < 2)
    {
        static const char* names[] = {
            "Call source",
            "Call source name"
        };
        return QVariant(names[section]);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool SymbolsTreeModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int SymbolsTreeModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

int SymbolsTreeModel::rowCount(const QModelIndex& parent) const
{
    GenericTreeNode* node = rootNode.get();
    if (parent.isValid())
    {
        node = static_cast<GenericTreeNode*>(parent.internalPointer());
    }
    return node->ChildrenCount();
}

QModelIndex SymbolsTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent))
    {
        GenericTreeNode* node = rootNode.get();
        if (parent.isValid())
        {
            node = static_cast<GenericTreeNode*>(parent.internalPointer());
        }
        if (node->ChildrenCount() > 0)
        {
            return createIndex(row, column, node->Child(row));
        }
    }
    return QModelIndex();
}

QModelIndex SymbolsTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        GenericTreeNode* node = static_cast<GenericTreeNode*>(index.internalPointer());
        if (rootNode.get() != node->Parent())
        {
            return createIndex(node->Index(), 0, node->Parent());
        }
    }
    return QModelIndex();
}

void SymbolsTreeModel::BuildTree()
{
    if (symbolMap.empty()) return;

    Vector<SymbolMapType::const_iterator> v;
    v.reserve(symbolMap.size());

    for (auto i = symbolMap.cbegin(), e = symbolMap.cend();i != e;++i)
    {
        v.push_back(i);
    }
    std::sort(v.begin(), v.end(), [](const SymbolMapType::const_iterator& l, const SymbolMapType::const_iterator& r) -> bool {
        return l->second < r->second;
    });

    String s = "!@%$@&$";
    NameNode* name = nullptr;
    for (auto x : v)
    {
        if (x->second != s)
        {
            s = x->second;
            name = new NameNode(s);
            rootNode->AppendChild(name);
        }
        name->AppendChild(new AddrNode(x->first));
    }
}

QVariant SymbolsTreeModel::NameNode::Data(int row, int clm) const
{
    switch (clm)
    {
    case 0:
        return QVariant(name.c_str());
    default:
        return QVariant();
    }
}

QVariant SymbolsTreeModel::AddrNode::Data(int row, int clm) const
{
    char buf[32];
    switch (clm)
    {
    case 0:
        Snprintf(buf, COUNT_OF(buf), "%08llX", addr);
        return QVariant(buf);
    default:
        return QVariant();
    }
}
