#include "Debug/DVAssert.h"

#include "Qt/DeviceInfo/MemoryTool/Models/BranchDiffTreeModel.h"
#include "Qt/DeviceInfo/MemoryTool/Models/DataFormat.h"

#include "Qt/DeviceInfo/MemoryTool/MemorySnapshot.h"
#include "Qt/DeviceInfo/MemoryTool/Branch.h"
#include "Qt/DeviceInfo/MemoryTool/BranchDiff.h"

#include <QColor>

using namespace DAVA;

BranchDiffTreeModel::BranchDiffTreeModel(const MemorySnapshot* snapshot1_, const MemorySnapshot* snapshot2_, QObject* parent)
    : QAbstractItemModel(parent)
    , snapshot1(snapshot1_)
    , snapshot2(snapshot2_)
{
    DVASSERT(snapshot1 != nullptr && snapshot1->IsLoaded());
    DVASSERT(snapshot2 != nullptr && snapshot2->IsLoaded());
}

BranchDiffTreeModel::~BranchDiffTreeModel()
{
    SafeDelete(rootDiff);
    SafeDelete(rootLeft);
    SafeDelete(rootRight);
}

void BranchDiffTreeModel::PrepareModel(const Vector<const String*>& names)
{
    DVASSERT(!names.empty());

    beginResetModel();
    SafeDelete(rootDiff);
    SafeDelete(rootLeft);
    SafeDelete(rootRight);

    rootLeft = snapshot1->CreateBranch(names);
    rootRight = snapshot2->CreateBranch(names);
    rootDiff = BranchDiff::Create(rootLeft, rootRight);
    endResetModel();
}

void BranchDiffTreeModel::ResetModel()
{
    beginResetModel();
    SafeDelete(rootDiff);
    SafeDelete(rootLeft);
    SafeDelete(rootRight);
    endResetModel();
}

QVariant BranchDiffTreeModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && rootDiff != nullptr)
    {
        BranchDiff* branch = static_cast<BranchDiff*>(index.internalPointer());
        if (Qt::DisplayRole == role)
        {
            int clm = index.column();
            switch (clm)
            {
            case CLM_NAME:
                if (branch->left)
                    return QString(branch->left->name->c_str());
                else if (branch->right)
                    return QString(branch->right->name->c_str());
                else
                    return QString("Root");
            case CLM_STAT1:
                if (branch->left)
                {
                    return QString("alloc=%1, nblocks=%2")
                        .arg(FormatNumberWithDigitGroups(branch->left->allocByApp).c_str())
                        .arg(branch->left->nblocks);
                }
                break;
            case CLM_STAT2:
                if (branch->right)
                {
                    return QString("alloc=%1, nblocks=%2")
                        .arg(FormatNumberWithDigitGroups(branch->right->allocByApp).c_str())
                        .arg(branch->right->nblocks);
                }
                break;
            default:
                break;
            }
        }
        else if (Qt::BackgroundRole == role)
        {
            if (branch->left && !branch->right)
                return QColor(Qt::red);
            else if (branch->right && !branch->left)
                return QColor(Qt::green);
        }
    }
    return QVariant();
}

bool BranchDiffTreeModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int BranchDiffTreeModel::columnCount(const QModelIndex& parent) const
{
    return NCOLUMNS;
}

int BranchDiffTreeModel::rowCount(const QModelIndex& parent) const
{
    if (rootDiff != nullptr)
    {
        BranchDiff* branch = rootDiff;
        if (parent.isValid())
        {
            branch = static_cast<BranchDiff*>(parent.internalPointer());
        }
        return static_cast<int>(branch->children.size());
    }
    return 0;
}

QModelIndex BranchDiffTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent) && rootDiff != nullptr)
    {
        BranchDiff* parentBranch = rootDiff;
        if (parent.isValid())
        {
            parentBranch = static_cast<BranchDiff*>(parent.internalPointer());
        }
        if (!parentBranch->children.empty())
        {
            return createIndex(row, column, parentBranch->children[row]);
        }
    }
    return QModelIndex();
}

QModelIndex BranchDiffTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid() && rootDiff != nullptr)
    {
        BranchDiff* branch = static_cast<BranchDiff*>(index.internalPointer());
        if (rootDiff != branch->parent)
        {
            return createIndex(branch->level - 1, 0, branch->parent);
        }
    }
    return QModelIndex();
}
