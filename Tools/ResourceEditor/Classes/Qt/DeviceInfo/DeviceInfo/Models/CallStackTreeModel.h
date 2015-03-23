#ifndef __CALLSTACKTREEMODEL_H__
#define __CALLSTACKTREEMODEL_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "GenericTreeNode.h"

#include "GenericTreeModel.h"
#include "GenericTreeNode.h"

class BacktraceSet;
class CallStackTreeModel : public GenericTreeModel
{
    Q_OBJECT

public:
    enum {
        TYPE_FRAME = 1,
        TYPE_BLOCK
    };

    struct BlockGroup
    {
        size_t startFrame;
        const DAVA::MMBacktrace* bktrace;
        DAVA::Vector<const DAVA::MMBlock*> blocks;
        size_t allocByApp;
        size_t totalAlloc;
        size_t nblocks;
    };

    class FrameAddrNode : public GenericTreeNode
    {
    public:
        FrameAddrNode(DAVA::uint64 adr) : addr(adr), allocByApp(0), totalAlloc(0), nblocks(0) {}
        int Type() const override { return TYPE_FRAME; }
        DAVA::uint64 Address() const { return addr; }
        size_t AllocByApp() const { return allocByApp; }
        size_t TotalAlloc() const { return totalAlloc; }
        size_t BlockCount() const { return nblocks; }
        void UpdateAllocByApp(size_t n) { allocByApp += n; }
        void UpdateTotalAlloc(size_t n) { totalAlloc += n; }
        void UpdateBlockCount(size_t n) { nblocks += n; }
        FrameAddrNode* FindInChildren(DAVA::uint64 addr) const {
            for (auto& x : children)
            {
                FrameAddrNode* node = static_cast<FrameAddrNode*>(x.get());
                if (node->addr == addr)
                    return node;
            }
            return nullptr;
        }
    private:
        DAVA::uint64 addr;
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

public:
    CallStackTreeModel(const DAVA::Vector<DAVA::MMBlock>& memoryBlocks,
                       const BacktraceSet& backtrace,
                       QObject* parent = nullptr);
    virtual ~CallStackTreeModel();

public slots:
    void Rebuild(const DAVA::Vector<DAVA::uint64>& addrList, bool append);

public:
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void BuildTree(const DAVA::Vector<DAVA::uint64>& addrList);
    void CollectBlockGroups(DAVA::Vector<BlockGroup>& grp, const DAVA::Vector<DAVA::uint64>& addrList);
    void CollectBlocks(BlockGroup& g, DAVA::uint32 hash) const;
    void AddBlockGroup(FrameAddrNode* parent, const BlockGroup& g);
    void AddBlocks(FrameAddrNode* parent, const BlockGroup& g);
    size_t BacktraceFindAnyAddr(const DAVA::MMBacktrace& o, const DAVA::Vector<DAVA::uint64>& addrList) const;

    void BuildMap();

private:
    QVariant FrameAddrNodeData(FrameAddrNode* node, int row, int clm) const;
    QVariant BlockNodeData(BlockNode* node, int row, int clm) const;

private:
    const BacktraceSet& bktrace;
    const DAVA::Vector<DAVA::MMBlock>& blockList;

    DAVA::Map<DAVA::uint32, DAVA::Vector<const DAVA::MMBlock*>> map;

    std::unique_ptr<FrameAddrNode> rootNode;
};

#endif  // __CALLSTACKTREEMODEL_H__
