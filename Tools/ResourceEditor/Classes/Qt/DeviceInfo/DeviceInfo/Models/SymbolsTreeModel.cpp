#include <QDebug>

#include "../BacktraceSet.h"
#include "SymbolsTreeModel.h"

using namespace DAVA;

SymbolsTreeModel::SymbolsTreeModel(const BacktraceSet& backtrace,
                                   QObject* parent)
    : GenericTreeModel(1, parent)
    , bktrace(backtrace)
    , rootNode(new GenericTreeNode)
{
    SetRootNode(rootNode.get());
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
        switch (node->Type())
        {
        case TYPE_NAME:
            return NameNodeData(static_cast<NameNode*>(node), index.row(), index.column());
        case TYPE_ADDR:
            return AddrNodeData(static_cast<AddrNode*>(node), index.row(), index.column());
        }
    }
    return QVariant();
}

QVariant SymbolsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    /*if (Qt::Horizontal == orientation && Qt::DisplayRole == role && 0 <= section && section < 2)
    {
        static const char* names[] = {
            "Call source",
            "Call source name"
        };
        return QVariant(names[section]);
    }*/
    return QAbstractItemModel::headerData(section, orientation, role);
}

void SymbolsTreeModel::BuildTree()
{
    if (bktrace.SymbolsEmpty()) return;

    for (auto i = bktrace.UniqueNamesCBegin(), e = bktrace.UniqueNamesCEnd();i != e;++i)
    {
        const char8* s = i->c_str();
        rootNode->AppendChild(new NameNode(s));
    }
    /*Set<const char8*> set;
    for (auto i = bktrace.BacktraceCBegin(), e = bktrace.BacktraceCEnd();i != e;++i)
    {
        const MMBacktrace& b = i->second;
        for (size_t j = 0, n = b.depth;j < n;++j)
            set.insert(reinterpret_cast<const char8*>(b.frames[j]));
    }

    for (auto i = set.begin(), e = set.end();i != e;++i)
    {
        const char8* s = *i;
        rootNode->AppendChild(new NameNode(s));
    }*/
    /*Vector<BacktraceSet::SymbolMapConstIterator> v;
    v.reserve(bktrace.SymbolsCount());

    for (auto i = bktrace.SymbolsCBegin(), e = bktrace.SymbolsCEnd();i != e;++i)
    {
        v.push_back(i);
    }
    std::sort(v.begin(), v.end(), [](const BacktraceSet::SymbolMapConstIterator l, const BacktraceSet::SymbolMapConstIterator r) -> bool {
        return l->second < r->second;
    });

    String s = "some dummy string";
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
    }*/
}

QVariant SymbolsTreeModel::NameNodeData(NameNode* node, int row, int clm) const
{
    return QVariant(node->Name());
    //return QVariant(node->Name().c_str());
}

QVariant SymbolsTreeModel::AddrNodeData(AddrNode* node, int row, int clm) const
{
    char buf[32];
    Snprintf(buf, COUNT_OF(buf), "%08llX", node->Address());
    return QVariant(buf);
}
