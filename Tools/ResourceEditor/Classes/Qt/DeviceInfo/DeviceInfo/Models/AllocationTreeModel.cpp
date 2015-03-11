#include "AllocationTreeModel.h"

using namespace DAVA;

AllocationTreeModel::AllocationTreeModel(const Vector<MMBlock>& memoryBlocks,
                                         const SymbolMapType& symbols,
                                         const BacktraceMapType& backtraces,
                                         QObject* parent)
    : QAbstractItemModel(parent)
    , symbolMap(symbols)
    , backtraceMap(backtraces)
    , blocks(memoryBlocks)
    , rootNode(new GenericTreeNode)
{
    BuildTree();
}

AllocationTreeModel::~AllocationTreeModel()
{

}

QVariant AllocationTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role)
    {
        GenericTreeNode* node = static_cast<GenericTreeNode*>(index.internalPointer());
        return node->Data(index.row(), index.column());
    }
    return QVariant();
}

QVariant AllocationTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Horizontal == orientation && Qt::DisplayRole == role && 0 <= section && section < 2)
    {
        static const char* names[] = {
            "Item",
            "Description",
        };
        return QVariant(names[section]);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool AllocationTreeModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int AllocationTreeModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

int AllocationTreeModel::rowCount(const QModelIndex& parent) const
{
    GenericTreeNode* node = rootNode.get();
    if (parent.isValid())
    {
        node = static_cast<GenericTreeNode*>(parent.internalPointer());
    }
    return node->ChildrenCount();
}

QModelIndex AllocationTreeModel::index(int row, int column, const QModelIndex& parent) const
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

QModelIndex AllocationTreeModel::parent(const QModelIndex& index) const
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

void AllocationTreeModel::BuildTree()
{
    // Sort memory blocks by pool index, then by descending allocation size, then by order number
    std::sort(blocks.begin(), blocks.end(), [](const MMBlock& l, const MMBlock& r) -> bool {
        // I know you like it
        return l.pool == r.pool ? l.allocByApp == r.allocByApp ? l.orderNo < r.orderNo
                                                               : r.allocByApp < l.allocByApp
                                : l.pool < r.pool;

    });

    AllocPoolInfo curPool(blocks.front().pool);
    auto begin = blocks.cbegin();
    auto end = blocks.cend();
    for (auto i = begin;i != end;++i)
    {
        const MMBlock& curBlock = *i;
        if (curPool.poolIndex == curBlock.pool)
        {
            curPool.allocByApp += curBlock.allocByApp;
            curPool.allocTotal += curBlock.allocTotal;
            curPool.nblocks += 1;
        }
        else
        {
            PoolNode* node = new PoolNode(curPool);
            BuildGroups(node, begin, i);
            rootNode->AppendChild(node);

            curPool = AllocPoolInfo(curBlock.pool);
            curPool.allocByApp = curBlock.allocByApp;
            curPool.allocTotal = curBlock.allocTotal;
            curPool.nblocks = 1;

            begin = i;
        }
    }
    {
        PoolNode* node = new PoolNode(curPool);
        BuildGroups(node, begin, end);
        rootNode->AppendChild(node);
    }
}

void AllocationTreeModel::BuildGroups(GenericTreeNode* parentNode, DAVA::Vector<DAVA::MMBlock>::const_iterator begin, DAVA::Vector<DAVA::MMBlock>::const_iterator end)
{
    BlockGroupInfo curGroup(begin->allocByApp);
    for (auto i = begin;i != end;++i)
    {
        const MMBlock& curBlock = *i;
        if (curGroup.allocSize == i->allocByApp)
        {
            curGroup.allocByApp += curBlock.allocByApp;
            curGroup.allocTotal += curBlock.allocTotal;
            curGroup.nblocks += 1;
        }
        else
        {
            GroupNode* node = new GroupNode(curGroup);
            BuildBlocks(node, begin, i);
            parentNode->AppendChild(node);

            curGroup = BlockGroupInfo(curBlock.allocByApp);
            curGroup.allocByApp = curBlock.allocByApp;
            curGroup.allocTotal = curBlock.allocTotal;
            curGroup.nblocks = 1;

            begin = i;
        }
    }
    {
        GroupNode* node = new GroupNode(curGroup);
        BuildBlocks(node, begin, end);
        parentNode->AppendChild(node);
    }
}

//////////////////////////////////////////////////////////////////////////
static const MMBacktrace* BacktraceGetBacktrace(const AllocationTreeModel::BacktraceMapType& backtraceMap, uint32 hash)
{
    auto i = backtraceMap.find(hash);
    return i != backtraceMap.cend() ? &i->second
                                    : nullptr;
}

static size_t BacktraceFrameCount(const AllocationTreeModel::BacktraceMapType& backtraceMap, uint32 hash)
{
    size_t n = 0;
    const MMBacktrace* backtrace = BacktraceGetBacktrace(backtraceMap, hash);
    if (backtrace != nullptr)
    {
        while (n < MMConst::BACKTRACE_DEPTH && backtrace->frames[n] != 0)
            n += 1;
    }
    return n;
}

static const char* BacktraceGetSymbol(const AllocationTreeModel::SymbolMapType& symbolMap, uint64 addr)
{
    auto i = symbolMap.find(addr);
    return i != symbolMap.cend() ? i->second.c_str()
                                 : nullptr;
}
//////////////////////////////////////////////////////////////////////////

void AllocationTreeModel::BuildBlocks(GenericTreeNode* parentNode, DAVA::Vector<DAVA::MMBlock>::const_iterator begin, DAVA::Vector<DAVA::MMBlock>::const_iterator end)
{
    for (;begin != end;++begin)
    {
        const MMBlock& curBlock = *begin;
        size_t nframes = BacktraceFrameCount(backtraceMap, curBlock.backtraceHash);
        BlockNode* node = new BlockNode(curBlock, nframes);
        {
            BacktraceNode* child = new BacktraceNode(curBlock.backtraceHash, symbolMap, backtraceMap);
            node->AppendChild(child);
        }
        parentNode->AppendChild(node);
    }
}

QVariant AllocationTreeModel::PoolNode::Data(int row, int clm) const
{
    switch (clm)
    {
    case 0:
        return QString("pool #%1").arg(pool.poolIndex);
    case 1:
        return QString("%1 bytes allocated in %2 blocks (total overhead %3 bytes)")
            .arg(pool.allocByApp)
            .arg(pool.nblocks)
            .arg(pool.allocTotal - pool.allocByApp);
    /*case 1:
        return QVariant(pool.allocByApp);
    case 2:
        return QVariant(pool.allocTotal);
    case 3:
        return QVariant(pool.nblocks);*/
    default:
        return QVariant();
    }
}

QVariant AllocationTreeModel::GroupNode::Data(int row, int clm) const
{
    switch (clm)
    {
    case 0:
        return QString("%1:").arg(group.allocSize);
    case 1:
        return QString("%1 bytes allocated in %2 blocks (total overhead %3 bytes)")
            .arg(group.allocByApp)
            .arg(group.nblocks)
            .arg(group.allocTotal - group.allocByApp);
    /*case 1:
        return QVariant(group.allocByApp);
    case 2:
        return QVariant(group.allocTotal);
    case 3:
        return QVariant(group.nblocks);*/
    default:
        return QVariant();
    }
}

QVariant AllocationTreeModel::BlockNode::Data(int row, int clm) const
{
    char buf[32];
    switch (clm)
    {
    case 0:
        return QString("order %1").arg(block.orderNo);
    case 1:
        Snprintf(buf, COUNT_OF(buf), "%08llX", block.addr);
        return QString("at address %1").arg(buf);
    /*case 0:
        return QVariant(block.orderNo);
    case 1:
        return QVariant(block.allocByApp);
    case 2:
        return QVariant(block.allocTotal);
    case 3:
        Snprintf(buf, COUNT_OF(buf), "%08llX", block.addr);
        return QVariant(buf);*/
    default:
        return QVariant();
    }
}

QVariant AllocationTreeModel::BacktraceNode::Data(int row, int clm) const
{
    char buf[32];
    const MMBacktrace* backtrace = BacktraceGetBacktrace(backtraceMap, hash);
    switch (clm)
    {
    case 0:
        Snprintf(buf, COUNT_OF(buf), "%08llX", backtrace->frames[row]);
        return QVariant(buf);
    case 1:
        return QVariant(BacktraceGetSymbol(symbolMap, backtrace->frames[row]));
    default:
        return QVariant();
    }
}
