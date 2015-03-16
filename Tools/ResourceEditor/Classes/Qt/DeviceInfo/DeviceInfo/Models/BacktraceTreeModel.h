#ifndef __BACKTRACETREEMODEL_H__
#define __BACKTRACETREEMODEL_H__

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "GenericTreeNode.h"

class BacktraceTreeModel : public QAbstractItemModel
{
public:
    using SymbolMapType = DAVA::UnorderedMap < DAVA::uint64, DAVA::String > ;
    using BacktraceMapType = DAVA::UnorderedMap < DAVA::uint32, DAVA::MMBacktrace > ;

private:
    class FrameAddrNode : public GenericTreeNode
    {
    public:
        FrameAddrNode(DAVA::uint64 addr, const SymbolMapType& symbols);
        QVariant Data(int row, int clm) const override;
        DAVA::uint64 Address() const;
        FrameAddrNode* FindInChildren(DAVA::uint64 childAddr) const;
    private:
        DAVA::uint64 addr;
        const SymbolMapType& symbolMap;
    };

public:
    BacktraceTreeModel(const DAVA::Vector<DAVA::MMBlock>& memoryBlocks,
                       const SymbolMapType& symbols,
                       const BacktraceMapType& backtraces,
                       QObject* parent = nullptr);
    virtual ~BacktraceTreeModel();

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    void BuildTree();
    void AddBacktrace(const DAVA::MMBacktrace& backtrace);

    const DAVA::MMBacktrace* GetBacktrace(DAVA::uint32 hash) const;

private:
    const SymbolMapType& symbolMap;
    const BacktraceMapType& backtraceMap;

    DAVA::Vector<DAVA::MMBlock> blocks;
    std::unique_ptr<FrameAddrNode> rootNode;
};

//////////////////////////////////////////////////////////////////////////
inline BacktraceTreeModel::FrameAddrNode::FrameAddrNode(DAVA::uint64 anAddr, const SymbolMapType& symbols)
    : addr(anAddr)
    , symbolMap(symbols)
{}

inline DAVA::uint64 BacktraceTreeModel::FrameAddrNode::Address() const
{
    return addr;
}

#endif  // __BACKTRACETREEMODEL_H__
