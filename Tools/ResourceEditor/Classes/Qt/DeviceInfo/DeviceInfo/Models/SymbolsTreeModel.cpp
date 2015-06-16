#include <QDebug>

#include "../BacktraceSymbolTable.h"
#include "SymbolsTreeModel.h"

using namespace DAVA;

SymbolsTreeModel::SymbolsTreeModel(const BacktraceSymbolTable& backtraceTable,
                                   QObject* parent)
    : GenericTreeModel(1, parent)
    , bktraceTable(backtraceTable)
    , rootNode(new GenericTreeNode)
{
    SetRootNode(rootNode.get());
    BuildTree();
}

SymbolsTreeModel::~SymbolsTreeModel()
{

}

QVariant SymbolsTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role)
    {
        GenericTreeNode* node = static_cast<GenericTreeNode*>(index.internalPointer());
        switch (node->Type())
        {
        case TYPE_NAME:
            return NameNodeData(static_cast<NameNode*>(node), index.row(), index.column());
        }
    }
    return QVariant();
}

void SymbolsTreeModel::BuildTree()
{
    bktraceTable.IterateOverSymbols([this](const char8* name) -> void {
        rootNode->AppendChild(new NameNode(name));
    });
}

QVariant SymbolsTreeModel::NameNodeData(NameNode* node, int row, int clm) const
{
    return QVariant(node->Name());
}
