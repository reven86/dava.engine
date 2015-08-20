#include "Debug/DVAssert.h"

#include "Qt/DeviceInfo/MemoryTool/Models/BranchTreeModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/DataFormat.h"

#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"

#include <QColor>

using namespace DAVA;

BranchTreeModel::BranchTreeModel(const MemorySnapshot* snapshot_, QObject* parent)
    : QAbstractItemModel(parent)
    , snapshot(snapshot_)
{
    DVASSERT(snapshot != nullptr && snapshot->IsLoaded());
}

BranchTreeModel::~BranchTreeModel()
{
    SafeDelete(rootBranch);
}

void BranchTreeModel::PrepareModel(const Vector<const String*>& names)
{
    DVASSERT(!names.empty());

    beginResetModel();
    SafeDelete(rootBranch);
    rootBranch = snapshot->CreateBranch(names);
    endResetModel();
}

void BranchTreeModel::ResetModel()
{
    beginResetModel();
    SafeDelete(rootBranch);
    endResetModel();
}

QVariant BranchTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && rootBranch != nullptr)
    {
        Branch* branch = static_cast<Branch*>(index.internalPointer());
        if (Qt::DisplayRole == role)
        {
            int clm = index.column();
            switch (clm)
            {
            case CLM_NAME:
                return QString(branch->name != nullptr ? branch->name->c_str() : "Root");
            case CLM_STAT:
                return QString("alloc=%1, nblocks=%2")
                    .arg(FormatNumberWithDigitGroups(branch->allocByApp).c_str())
                    .arg(branch->nblocks);
            default:
                break;
            }
        }
    }
    return QVariant();
}

bool BranchTreeModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int BranchTreeModel::columnCount(const QModelIndex& parent) const
{
    return NCOLUMNS;
}

int BranchTreeModel::rowCount(const QModelIndex& parent) const
{
    if (rootBranch != nullptr)
    {
        Branch* branch = rootBranch;
        if (parent.isValid())
        {
            branch = static_cast<Branch*>(parent.internalPointer());
        }
        return static_cast<int>(branch->children.size());
    }
    return 0;
}

QModelIndex BranchTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent) && rootBranch != nullptr)
    {
        Branch* parentBranch = rootBranch;
        if (parent.isValid())
        {
            parentBranch = static_cast<Branch*>(parent.internalPointer());
        }
        if (!parentBranch->children.empty())
        {
            return createIndex(row, column, parentBranch->children[row]);
        }
    }
    return QModelIndex();
}

QModelIndex BranchTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid() && rootBranch != nullptr)
    {
        Branch* branch = static_cast<Branch*>(index.internalPointer());
        if (rootBranch != branch->parent)
        {
            return createIndex(branch->level - 1, 0, branch->parent);
        }
    }
    return QModelIndex();
}
