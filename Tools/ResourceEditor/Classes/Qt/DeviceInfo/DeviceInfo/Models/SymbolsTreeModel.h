#pragma once

#include "Base/BaseTypes.h"

#include "GenericTreeModel.h"
#include "GenericTreeNode.h"

class BacktraceSet;
class SymbolsTreeModel : public GenericTreeModel
{
public:
    enum {
        TYPE_NAME = 1,
        TYPE_ADDR
    };

    /*class NameNode : public GenericTreeNode
    {
    public:
        NameNode(const DAVA::String& s) : GenericTreeNode(TYPE_NAME), name(s) {}
        const DAVA::String& Name() const { return name; }
    private:
        DAVA::String name;
    };*/

    class NameNode : public GenericTreeNode
    {
    public:
        NameNode(const DAVA::char8* s) : GenericTreeNode(TYPE_NAME), name(s) {}
        const DAVA::char8* Name() const { return name; }
        DAVA::uint64 Address() const { return reinterpret_cast<DAVA::uint64>(name); }
    private:
        const DAVA::char8* name;
    };

    class AddrNode : public GenericTreeNode
    {
    public:
        AddrNode(DAVA::uint64 a) : GenericTreeNode(TYPE_ADDR), addr(a) {}
        DAVA::uint64 Address() const { return addr; }
    private:
        DAVA::uint64 addr;
    };

public:
    SymbolsTreeModel(const BacktraceSet& backtrace, QObject* parent = nullptr);
    virtual ~SymbolsTreeModel();

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void BuildTree();

    QVariant NameNodeData(NameNode* node, int row, int clm) const;
    QVariant AddrNodeData(AddrNode* node, int row, int clm) const;

private:
    const BacktraceSet& bktrace;

    std::unique_ptr<GenericTreeNode> rootNode;
};
