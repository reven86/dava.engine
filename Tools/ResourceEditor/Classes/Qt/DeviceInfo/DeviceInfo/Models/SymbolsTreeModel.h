#pragma once

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

#include "GenericTreeNode.h"

class SymbolsTreeModel : public QAbstractItemModel
{
public:
    using SymbolMapType = DAVA::UnorderedMap < DAVA::uint64, DAVA::String > ;
    using BacktraceMapType = DAVA::UnorderedMap < DAVA::uint32, DAVA::MMBacktrace > ;

    class NameNode : public GenericTreeNode
    {
    public:
        NameNode(const DAVA::String& s) : name(s) {}
        QVariant Data(int row, int clm) const override;
    private:
        DAVA::String name;
    };

    class AddrNode : public GenericTreeNode
    {
    public:
        AddrNode(DAVA::uint64 a) : addr(a) {}
        QVariant Data(int row, int clm) const override;
    private:
        DAVA::uint64 addr;
    };

public:
    SymbolsTreeModel(const SymbolMapType& symbols,
                     QObject* parent = nullptr);
    virtual ~SymbolsTreeModel();

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    void BuildTree();

private:
    const SymbolMapType& symbolMap;

    std::unique_ptr<GenericTreeNode> rootNode;
};
