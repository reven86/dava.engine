#ifndef __ALLOCATIONTREEMODEL_H__
#define __ALLOCATIONTREEMODEL_H__

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "GenericTreeNode.h"

class AllocationTreeModel : public QAbstractItemModel
{
public:
    using SymbolMapType = DAVA::UnorderedMap<DAVA::uint64, DAVA::String>;
    using BacktraceMapType = DAVA::UnorderedMap<DAVA::uint32, DAVA::MMBacktrace>;

private:
    struct AllocPoolInfo
    {
        AllocPoolInfo(DAVA::uint32 poolIndex);
        DAVA::uint32 poolIndex;
        size_t allocByApp;
        size_t allocTotal;
        size_t nblocks;
        size_t ngroups;
    };

    struct BlockGroupInfo
    {
        BlockGroupInfo(DAVA::uint32 allocSize);
        DAVA::uint32 allocSize;
        size_t allocByApp;
        size_t allocTotal;
        size_t nblocks;
    };

    class PoolNode : public GenericTreeNode
    {
    public:
        PoolNode(const AllocPoolInfo& poolInfo);
        QVariant Data(int row, int clm) const override;
    private:
        AllocPoolInfo pool;
    };

    class GroupNode : public GenericTreeNode
    {
    public:
        GroupNode(const BlockGroupInfo& groupInfo);
        QVariant Data(int row, int clm) const override;
    private:
        BlockGroupInfo group;
    };

    class BlockNode : public GenericTreeNode
    {
    public:
        BlockNode(const DAVA::MMBlock& aBlock, size_t nframes);
        int ChildrenCount() const override;
        GenericTreeNode* Child(int index) const override;
        QVariant Data(int row, int clm) const override;
    private:
        const DAVA::MMBlock& block;
        size_t nframes;
    };

    class BacktraceNode : public GenericTreeNode
    {
    public:
        BacktraceNode(DAVA::uint32 hash, const SymbolMapType& symbols, const BacktraceMapType& backtraces);
        QVariant Data(int row, int clm) const override;
    private:
        DAVA::uint32 hash;
        const SymbolMapType& symbolMap;
        const BacktraceMapType& backtraceMap;
    };

public:
    AllocationTreeModel(const DAVA::Vector<DAVA::MMBlock>& memoryBlocks,
                        const SymbolMapType& symbols,
                        const BacktraceMapType& backtraces,
                        QObject* parent = nullptr);
    virtual ~AllocationTreeModel();

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    void BuildTree();
    void BuildGroups(GenericTreeNode* parentNode, DAVA::Vector<DAVA::MMBlock>::const_iterator begin, DAVA::Vector<DAVA::MMBlock>::const_iterator end);
    void BuildBlocks(GenericTreeNode* parentNode, DAVA::Vector<DAVA::MMBlock>::const_iterator begin, DAVA::Vector<DAVA::MMBlock>::const_iterator end);

private:
    const SymbolMapType& symbolMap;
    const BacktraceMapType& backtraceMap;

    DAVA::Vector<DAVA::MMBlock> blocks;
    std::unique_ptr<GenericTreeNode> rootNode;
};

//////////////////////////////////////////////////////////////////////////
inline AllocationTreeModel::AllocPoolInfo::AllocPoolInfo(DAVA::uint32 aPoolIndex)
    : poolIndex(aPoolIndex)
    , allocByApp(0)
    , allocTotal(0)
    , nblocks(0)
    , ngroups(0)
{}

inline AllocationTreeModel::BlockGroupInfo::BlockGroupInfo(DAVA::uint32 anAllocSize)
    : allocSize(anAllocSize)
    , allocByApp(0)
    , allocTotal(0)
    , nblocks(0)
{}

inline AllocationTreeModel::PoolNode::PoolNode(const AllocPoolInfo& poolInfo)
    : GenericTreeNode()
    , pool(poolInfo)
{}

inline AllocationTreeModel::GroupNode::GroupNode(const BlockGroupInfo& groupInfo)
    : GenericTreeNode()
    , group(groupInfo)
{}

inline AllocationTreeModel::BlockNode::BlockNode(const DAVA::MMBlock& aBlock, size_t frameCount)
    : GenericTreeNode()
    , block(aBlock)
    , nframes(frameCount)
{}

inline int AllocationTreeModel::BlockNode::ChildrenCount() const
{
    return static_cast<int>(nframes);
}

inline GenericTreeNode* AllocationTreeModel::BlockNode::Child(int index) const
{
    return children.front().get();  // BlockNode has only one child, return it
}

inline AllocationTreeModel::BacktraceNode::BacktraceNode(DAVA::uint32 aHash, const SymbolMapType& symbols, const BacktraceMapType& backtraces)
    : GenericTreeNode()
    , hash(aHash)
    , symbolMap(symbols)
    , backtraceMap(backtraces)
{}

#endif  // __ALLOCATIONTREEMODEL_H__
