#include "GenericTreeNode.h"
#include "GenericTreeModel.h"

using namespace DAVA;

GenericTreeModel::GenericTreeModel(int columnCount, QObject* parent)
    : QAbstractItemModel(parent)
    , rootNode(nullptr)
    , ncolumns(columnCount)
{

}

GenericTreeModel::~GenericTreeModel()
{

}

bool GenericTreeModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int GenericTreeModel::columnCount(const QModelIndex& parent) const
{
    return ncolumns;
}

int GenericTreeModel::rowCount(const QModelIndex& parent) const
{
    GenericTreeNode* node = rootNode;
    if (parent.isValid())
    {
        node = static_cast<GenericTreeNode*>(parent.internalPointer());
    }
    return node->ChildrenCount();
}

QModelIndex GenericTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent))
    {
        GenericTreeNode* parentNode = rootNode;
        if (parent.isValid())
        {
            parentNode = static_cast<GenericTreeNode*>(parent.internalPointer());
        }
        if (parentNode->ChildrenCount() > 0)
        {
            return createIndex(row, column, parentNode->Child(row));
        }
    }
    return QModelIndex();
}

QModelIndex GenericTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid())
    {
        GenericTreeNode* node = static_cast<GenericTreeNode*>(index.internalPointer());
        if (rootNode != node->Parent())
        {
            return createIndex(node->Index(), 0, node->Parent());
        }
    }
    return QModelIndex();
}

void GenericTreeModel::SetRootNode(GenericTreeNode* root)
{
    rootNode = root;
}
