#include "../BacktraceSet.h"
#include "CallStackTreeModel.h"

using namespace DAVA;

CallStackTreeModel::CallStackTreeModel(const DAVA::Vector<DAVA::MMBlock>& memBlocks,
                                       const BacktraceSet& backtrace,
                                       QObject* parent)
    : GenericTreeModel(2, parent)
    , bktrace(backtrace)
    , blockList(memBlocks)
    , rootNode(new FrameAddrNode(0))
{
    SetRootNode(rootNode.get());
}

CallStackTreeModel::~CallStackTreeModel()
{

}

void CallStackTreeModel::Rebuild(const DAVA::Vector<DAVA::uint64>& addrList, bool append)
{
    beginResetModel();
    if (!append)
    {
        rootNode.reset(new FrameAddrNode(0));
    }
    SetRootNode(rootNode.get());
    BuildTree(addrList);
    endResetModel();
}

QVariant CallStackTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role)
    {
        GenericTreeNode* node = static_cast<GenericTreeNode*>(index.internalPointer());
        int type = node->Type();
        if (type == TYPE_FRAME)
            return FrameAddrNodeData(static_cast<FrameAddrNode*>(node), index.row(), index.column());
        else if (type == TYPE_BLOCK)
            return BlockNodeData(static_cast<BlockNode*>(node), index.row(), index.column());
    }
    return QVariant();
}

QVariant CallStackTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Horizontal == orientation && Qt::DisplayRole == role && 0 <= section && section < 3)
    {
        static const char* names[] = {
            "1",
            "2",
            "3"
        };
        return QVariant(names[section]);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

void CallStackTreeModel::BuildTree(const DAVA::Vector<DAVA::uint64>& addrList)
{
    Vector<BlockGroup> grp;
    CollectBlockGroups(grp, addrList);

    size_t a = 0;
    size_t t = 0;
    size_t n = 0;

    FrameAddrNode* root = rootNode.get();
    FrameAddrNode* tmp = new FrameAddrNode(-1);
    root->AppendChild(tmp);

    size_t n1 = grp.size();
    size_t c = 0;
    for (auto& g : grp)
    {
        //AddBlockGroup(root, g);
        AddBlockGroup(tmp, g);
        a += g.allocByApp;
        t += g.totalAlloc;
        n += g.nblocks;
        c++;
    }
    tmp->UpdateAllocByApp(a);
    tmp->UpdateTotalAlloc(t);
    tmp->UpdateBlockCount(n);
}

void CallStackTreeModel::CollectBlockGroups(DAVA::Vector<BlockGroup>& grp, const DAVA::Vector<DAVA::uint64>& addrList)
{
    size_t n = bktrace.BacktraceCount();
    size_t c = 0;
    for (auto i = bktrace.BacktraceCBegin(), e = bktrace.BacktraceCEnd();i != e;++i, ++c)
    {
        const MMBacktrace& bk = i->second;
        size_t index = BacktraceFindAnyAddr(bk, addrList);
        if (index != size_t(-1))
        {
            BlockGroup g;
            g.allocByApp = 0;
            g.totalAlloc = 0;
            g.nblocks = 0;
            g.bktrace = &bk;
            g.startFrame = index;
            CollectBlocks(g, bk.hash);
            grp.push_back(g);
        }
    }
}

void CallStackTreeModel::CollectBlocks(BlockGroup& g, DAVA::uint32 hash) const
{
    for (auto& x : blockList)
    {
        if (x.backtraceHash == hash)
        {
            g.allocByApp += x.allocByApp;
            g.totalAlloc += x.allocTotal;
            g.nblocks += 1;
            g.blocks.push_back(&x);
        }
    }
}

void CallStackTreeModel::AddBlockGroup(FrameAddrNode* parent, const BlockGroup& g)
{
    size_t i = g.startFrame;
    do {
        uint64 addr = g.bktrace->frames[i];
        FrameAddrNode* child = parent->FindInChildren(addr);
        if (child == nullptr)
        {
            child = new FrameAddrNode(addr);
            parent->AppendChild(child);
        }
        child->UpdateAllocByApp(g.allocByApp);
        child->UpdateTotalAlloc(g.totalAlloc);
        child->UpdateBlockCount(g.nblocks);

        parent = child;
    } while (i-- > 0);
    if (g.startFrame > 0)
        AddBlocks(parent, g);
}

void CallStackTreeModel::AddBlocks(FrameAddrNode* parent, const BlockGroup& g)
{
    for (auto x : g.blocks)
    {
        parent->AppendChild(new BlockNode(x));
    }
    parent->SortChildren([](GenericTreeNode* l, GenericTreeNode* r) -> bool {
        BlockNode* bl = static_cast<BlockNode*>(l);
        BlockNode* br = static_cast<BlockNode*>(r);
        return bl->Block()->orderNo < br->Block()->orderNo;
    });
}

size_t CallStackTreeModel::BacktraceFindAnyAddr(const DAVA::MMBacktrace& o, const DAVA::Vector<DAVA::uint64>& addrList) const
{
    const uint64* addrBegin = &o.frames[MMConst::BACKTRACE_DEPTH - 1];
    const uint64* addrEnd = &o.frames[0] - 1;
    while (addrBegin > addrEnd && *addrBegin == 0)
        addrBegin -= 1;
    while (addrBegin > addrEnd)
    {
        auto i = std::find(addrList.begin(), addrList.end(), *addrBegin);
        if (i != addrList.end())
        {
            return size_t(addrBegin - &o.frames[0]);
        }
        addrBegin -= 1;
    }
    return size_t(-1);
}

QVariant CallStackTreeModel::FrameAddrNodeData(FrameAddrNode* node, int row, int clm) const
{
    char buf[32];
    switch (clm)
    {
    case 0:
        Snprintf(buf, COUNT_OF(buf), "%08llX", node->Address());
        return QString("%1 %2").arg(buf).arg(bktrace.GetSymbol(node->Address()));
    case 1:
        return QString("allocated %1 bytes in %2 blocks").arg(node->AllocByApp()).arg(node->BlockCount());
    default:
        return QVariant();
    }
}

QVariant CallStackTreeModel::BlockNodeData(BlockNode* node, int row, int clm) const
{
    switch (clm)
    {
    case 0:
        return QString("block=%1 of size %2")
            .arg(node->Block()->orderNo)
            .arg(node->Block()->allocByApp);
    default:
        return QVariant();
    }
}
