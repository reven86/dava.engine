#include "../BacktraceSymbolTable.h"
#include "CallStackTreeModel.h"

using namespace DAVA;

CallStackTreeModel::CallStackTreeModel(const DAVA::Vector<DAVA::MMBlock>& memBlocks,
                                       const BacktraceSymbolTable& backtraceTable,
                                       QObject* parent)
    : GenericTreeModel(2, parent)
    , bktraceTable(backtraceTable)
    , blockList(memBlocks)
    , rootNode(new FrameAddrNode(0))
{
    BuildMap();
    SetRootNode(rootNode.get());
}

CallStackTreeModel::~CallStackTreeModel()
{

}

void CallStackTreeModel::Rebuild(const DAVA::Vector<const DAVA::char8*>& nameList, bool append)
{
    beginResetModel();
    if (!append)
    {
        rootNode.reset(new FrameAddrNode(0));
    }
    SetRootNode(rootNode.get());
    BuildTree(nameList);
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
        else if (type == TYPE_BLOCK2)
            return Block2NodeData(static_cast<BlockNode2*>(node), index.row(), index.column());
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

void CallStackTreeModel::BuildTree(const DAVA::Vector<const DAVA::char8*>& nameList)
{
    Vector<BlockGroup> grp;
    CollectBlockGroups(grp, nameList);

    size_t a = 0;
    size_t t = 0;
    size_t n = 0;

    FrameAddrNode* root = rootNode.get();
    FrameAddrNode* tmp = new FrameAddrNode(0);
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

void CallStackTreeModel::CollectBlockGroups(DAVA::Vector<BlockGroup>& grp, const DAVA::Vector<const DAVA::char8*>& nameList)
{
    bktraceTable.IterateOverBacktraces([this, &grp, &nameList](uint32 hash, const Vector<const char8*>& v) -> void {
        size_t index = BacktraceFindAnyAddr(v, nameList);
        if (index != size_t(-1))
        {
            BlockGroup g;
            g.allocByApp = 0;
            g.totalAlloc = 0;
            g.nblocks = 0;
            g.frames = &v;
            g.startFrame = index;
            CollectBlocks(g, hash);
            grp.push_back(g);
        }
    });
}

void CallStackTreeModel::CollectBlocks(BlockGroup& g, DAVA::uint32 hash) const
{
    auto xx = map.find(hash);
    Q_ASSERT(xx != map.end());
    for (auto y : xx->second)
    {
        g.allocByApp += y->allocByApp;
        g.totalAlloc += y->allocTotal;
        g.nblocks += 1;
        g.blocks.push_back(y);
    }
}

void CallStackTreeModel::AddBlockGroup(FrameAddrNode* parent, BlockGroup& g)
{
    size_t i = g.startFrame;
    do {
        const char8* name = g.frames->at(i);
        FrameAddrNode* child = parent->FindInChildren(name);
        if (child == nullptr)
        {
            child = new FrameAddrNode(name);
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

void CallStackTreeModel::AddBlocks(FrameAddrNode* parent, BlockGroup& g)
{
    //parent->AppendChild(new BlockNode2(g.blocks));
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

size_t CallStackTreeModel::BacktraceFindAnyAddr(const DAVA::Vector<const DAVA::char8*>& frames, const DAVA::Vector<const DAVA::char8*>& nameList) const
{
    size_t n = frames.size() - 1;
    do {
        auto iterFind = std::find(nameList.begin(), nameList.end(), frames[n]);
        if (iterFind != nameList.end())
        {
            return n;
        }
    } while (n-- > 0);
    /*for (auto i = frames.rbegin(), e = frames.rend();i != e;++i, ++n)
    {
        auto iterFind = std::find(nameList.begin(), nameList.end(), *i);
        if (iterFind != nameList.end())
        {
            size_t k = std::distance(frames.rbegin(), i);
            return k;
        }
    }*/
    return size_t(-1);
}

QVariant CallStackTreeModel::FrameAddrNodeData(FrameAddrNode* node, int row, int clm) const
{
    const char8* name = nullptr;
    switch (clm)
    {
    case 0:
        name = node->Name();
        if (name != nullptr)
        {
            return QVariant(name);
        }
        else
        {
            return QString("Root");
        }
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

QVariant CallStackTreeModel::Block2NodeData(BlockNode2* node, int row, int clm) const
{
    const DAVA::Vector<const DAVA::MMBlock*>& v = node->Blocks();
    switch (clm)
    {
    case 0:
        return QString("block=%1 of size %2")
            .arg(v[row]->orderNo)
            .arg(v[row]->allocByApp);
    default:
        return QVariant();
    }
}

void CallStackTreeModel::BuildMap()
{
    for (auto& x : blockList)
    {
        auto ff = map.find(x.backtraceHash);
        if (ff == map.end())
        {
            ff = map.emplace(std::make_pair(x.backtraceHash, DAVA::Vector<const DAVA::MMBlock*>())).first;
        }
        ff->second.push_back(&x);
    }
}
