#pragma once

#include "Base/BaseTypes.h"

#include "GenericTreeModel.h"
#include "GenericTreeNode.h"

class BacktraceSymbolTable;
class SymbolsTreeModel : public GenericTreeModel
{
public:
    enum {
        TYPE_NAME = 1
    };

    class NameNode : public GenericTreeNode
    {
    public:
        NameNode(const DAVA::char8*& s) : GenericTreeNode(TYPE_NAME), name(s) {}
        const DAVA::char8* Name() const { return name; }
    private:
        const DAVA::char8* name;
    };

public:
    SymbolsTreeModel(const BacktraceSymbolTable& backtraceTable, QObject* parent = nullptr);
    virtual ~SymbolsTreeModel();

    QVariant data(const QModelIndex& index, int role) const override;

private:
    void BuildTree();

    QVariant NameNodeData(NameNode* node, int row, int clm) const;

private:
    const BacktraceSymbolTable& bktraceTable;
    std::unique_ptr<GenericTreeNode> rootNode;
};
