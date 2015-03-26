#ifndef __CALLSTACKTREEMODEL_H__
#define __CALLSTACKTREEMODEL_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "GenericTreeNode.h"

#include "GenericTreeModel.h"
#include "GenericTreeNode.h"

class BacktraceSymbolTable;
class CallStackTreeModel : public GenericTreeModel
{
    Q_OBJECT

public:
    enum {
        TYPE_FRAME = 1,
        TYPE_BLOCK,
        TYPE_BLOCK2
    };

    struct BlockGroup
    {
        size_t startFrame;
        const DAVA::Vector<const DAVA::char8*>* frames;
        DAVA::Vector<const DAVA::MMBlock*> blocks;
        size_t allocByApp;
        size_t totalAlloc;
        size_t nblocks;
    };

    class FrameAddrNode : public GenericTreeNode
    {
    public:
        FrameAddrNode(const DAVA::char8* adr) : name(adr), allocByApp(0), totalAlloc(0), nblocks(0) {}
        int Type() const override { return TYPE_FRAME; }
        const DAVA::char8* Name() const { return name; }
        size_t AllocByApp() const { return allocByApp; }
        size_t TotalAlloc() const { return totalAlloc; }
        size_t BlockCount() const { return nblocks; }
        void UpdateAllocByApp(size_t n) { allocByApp += n; }
        void UpdateTotalAlloc(size_t n) { totalAlloc += n; }
        void UpdateBlockCount(size_t n) { nblocks += n; }
        FrameAddrNode* FindInChildren(const DAVA::char8* addr) const {
            for (auto& x : children)
            {
                FrameAddrNode* node = static_cast<FrameAddrNode*>(x.get());
                if (node->name == addr)
                    return node;
            }
            return nullptr;
        }
    private:
        const DAVA::char8* name;
        size_t allocByApp;
        size_t totalAlloc;
        size_t nblocks;
    };

    class BlockNode : public GenericTreeNode
    {
    public:
        BlockNode(const DAVA::MMBlock* b) : block(b) {}
        int Type() const override { return TYPE_BLOCK; }
        const DAVA::MMBlock* Block() const { return block; }
    private:
        const DAVA::MMBlock* block;
    };

    class BlockNode2 : public GenericTreeNode
    {
    public:
        BlockNode2(DAVA::Vector<const DAVA::MMBlock*>& blocksX) : blocks(std::move(blocksX)) {}
        int Type() const override { return TYPE_BLOCK2; }
        const DAVA::Vector<const DAVA::MMBlock*>& Blocks() const { return blocks; }
    private:
        DAVA::Vector<const DAVA::MMBlock*> blocks;
    };

public:
    CallStackTreeModel(const DAVA::Vector<DAVA::MMBlock>& memoryBlocks,
                       const BacktraceSymbolTable& backtraceTable,
                       QObject* parent = nullptr);
    virtual ~CallStackTreeModel();

public slots:
    void Rebuild(const DAVA::Vector<const DAVA::char8*>& nameList, bool append);

public:
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void BuildTree(const DAVA::Vector<const DAVA::char8*>& nameList);
    void CollectBlockGroups(DAVA::Vector<BlockGroup>& grp, const DAVA::Vector<const DAVA::char8*>& nameList);
    void CollectBlocks(BlockGroup& g, DAVA::uint32 hash) const;
    void AddBlockGroup(FrameAddrNode* parent, BlockGroup& g);
    void AddBlocks(FrameAddrNode* parent, BlockGroup& g);

    size_t BacktraceFindAnyAddr(const DAVA::Vector<const DAVA::char8*>& frames, const DAVA::Vector<const DAVA::char8*>& nameList) const;

    void BuildMap();

private:
    QVariant FrameAddrNodeData(FrameAddrNode* node, int row, int clm) const;
    QVariant BlockNodeData(BlockNode* node, int row, int clm) const;
    QVariant Block2NodeData(BlockNode2* node, int row, int clm) const;

private:
    const BacktraceSymbolTable& bktraceTable;
    const DAVA::Vector<DAVA::MMBlock>& blockList;

    DAVA::Map<DAVA::uint32, DAVA::Vector<const DAVA::MMBlock*>> map;

    std::unique_ptr<FrameAddrNode> rootNode;
};

#endif  // __CALLSTACKTREEMODEL_H__
