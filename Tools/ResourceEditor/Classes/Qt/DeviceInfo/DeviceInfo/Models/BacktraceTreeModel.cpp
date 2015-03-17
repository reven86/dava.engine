#include <QDebug>
#include "BacktraceTreeModel.h"

using namespace DAVA;

BacktraceTreeModel::BacktraceTreeModel(const DAVA::Vector<DAVA::MMBlock>& memBlocks,
                                       const std::unordered_map<DAVA::uint64, DAVA::String>& symbols,
                                       const std::unordered_map<DAVA::uint32, DAVA::MMBacktrace>& backtraces,
                                       QObject* parent)
    : QAbstractItemModel(parent)
    , symbolMap(symbols)
    , backtraceMap(backtraces)
    , blocks(memBlocks)
    , rootNode(new FrameAddrNode(0, symbolMap))
{
    BuildTree();
}

BacktraceTreeModel::~BacktraceTreeModel()
{

}

QVariant BacktraceTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role)
    {
        GenericTreeNode* node = static_cast<GenericTreeNode*>(index.internalPointer());
        return node->Data(index.row(), index.column());
    }
    return QVariant();
}

QVariant BacktraceTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool BacktraceTreeModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int BacktraceTreeModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

int BacktraceTreeModel::rowCount(const QModelIndex& parent) const
{
    GenericTreeNode* node = rootNode.get();
    if (parent.isValid())
    {
        node = static_cast<GenericTreeNode*>(parent.internalPointer());
    }
    return node->ChildrenCount();
}

QModelIndex BacktraceTreeModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex BacktraceTreeModel::parent(const QModelIndex& index) const
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

void BacktraceTreeModel::BuildTree()
{
    /*for (auto& x : blocks)
    {

    }*/
    for (const auto& x : backtraceMap)
    {
        const MMBacktrace& o = x.second;
        AddBacktrace(o);
    }
}

void BacktraceTreeModel::AddBacktrace(const DAVA::MMBacktrace& backtrace)
{
    FrameAddrNode* parent = rootNode.get();
    for (size_t i = 0;i < MMConst::BACKTRACE_DEPTH && backtrace.frames[i] != 0;++i)
    {
        FrameAddrNode* child = parent->FindInChildren(backtrace.frames[i]);
        if (child == nullptr)
        {
            child = new FrameAddrNode(backtrace.frames[i], symbolMap);
            parent->AppendChild(child);
        }
        parent = child;
    }
}

const DAVA::MMBacktrace* BacktraceTreeModel::GetBacktrace(DAVA::uint32 hash) const
{
    auto i = backtraceMap.find(hash);
    if (i != backtraceMap.cend())
    {
        return &i->second;
    }
    return nullptr;
}

QVariant BacktraceTreeModel::FrameAddrNode::Data(int row, int clm) const
{
    /*auto i = symbolMap.find(addr);
    if (i != symbolMap.cend())
    {
        return QVariant(i->second.c_str());
    }
    else
    {
        char buf[32];
        Snprintf(buf, COUNT_OF(buf), "%08llX", addr);
        return QVariant(buf);
    }*/
    char buf[32];
    switch (clm)
    {
    case 0:
        Snprintf(buf, COUNT_OF(buf), "%08llX", addr);
        return QVariant(buf);
    case 1:
        {
            auto i = symbolMap.find(addr);
            if (i != symbolMap.cend())
            {
                return QVariant(i->second.c_str());
            }
            else
                return QVariant();
        }
    default:
        return QVariant();
    }
}

BacktraceTreeModel::FrameAddrNode* BacktraceTreeModel::FrameAddrNode::FindInChildren(DAVA::uint64 childAddr) const
{
    for (auto& x : children)
    {
        FrameAddrNode* o = static_cast<FrameAddrNode*>(x.get());
        if (o->addr == childAddr)
            return o;
    }
    return nullptr;
}
