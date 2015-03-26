#include "BlockListModel.h"

using namespace DAVA;

BlockListModel::BlockListModel(bool diff, QObject* parent)
    : QAbstractItemModel(parent)
    , diffModel(diff)
    , ncolumns(3)
{

}

BlockListModel::~BlockListModel()
{

}

void BlockListModel::PrepareModel(DAVA::Vector<const DAVA::MMBlock*>& v1)
{
    beginResetModel();

    v.clear();
    v.swap(v1);
    endResetModel();
}

void BlockListModel::PrepareDiffModel(DAVA::Vector<const DAVA::MMBlock*>& v1, DAVA::Vector<const DAVA::MMBlock*>& v2)
{
    beginResetModel();

    v.clear();
    size_t n = std::min(v1.size(), v2.size());
    if (n > 0)
    {
        v.reserve(n);
        std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), std::back_inserter(v), [](const MMBlock* l, const MMBlock* r) -> bool {
            return l->orderNo < r->orderNo;
        });
    }

    endResetModel();
}

const DAVA::MMBlock* BlockListModel::GetBlock(const QModelIndex& index) const
{
    if (index.isValid())
    {
        size_t row = index.row();
        if (row < v.size())
            return v[row];
    }
    return nullptr;
}

QVariant BlockListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && Qt::DisplayRole == role)
    {
        int row = index.row();
        int clm = index.column();
        const MMBlock* block = v[row];
        switch (clm)
        {
        case 0:
            return QVariant(row + 1);
        case 1:
            return QString("%1").arg(block->orderNo);
        case 2:
            return QString("%1").arg(block->allocByApp);
        }
    }
    return QVariant();
}

int BlockListModel::columnCount(const QModelIndex& parent) const
{
    return ncolumns;
}

bool BlockListModel::hasChildren(const QModelIndex& parent) const
{
    return rowCount(parent) > 0;
}

int BlockListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return static_cast<int>(v.size());
    return 0;
}

QModelIndex BlockListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (hasIndex(row, column, parent))
    {
        return createIndex(row, column, nullptr);
    }
    return QModelIndex();
}

QModelIndex BlockListModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}
