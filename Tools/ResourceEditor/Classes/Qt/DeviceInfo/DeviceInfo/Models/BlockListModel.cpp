#include "Debug/DVAssert.h"

#include "BlockListModel.h"
#include "DataFormat.h"

#include <QColor>

using namespace DAVA;

BlockListModel::BlockListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

BlockListModel::~BlockListModel()
{}

void BlockListModel::PrepareModel(Vector<const MMBlock*>& blocks)
{
    beginResetModel();
    mblocks = std::move(blocks);
    endResetModel();
}

void BlockListModel::ResetModel()
{
    beginResetModel();
    mblocks.clear();
    endResetModel();
}

const MMBlock* BlockListModel::GetBlock(const QModelIndex& index) const
{
    if (index.isValid())
    {
        int row = index.row();
        DVASSERT(static_cast<size_t>(row) < mblocks.size());
        return mblocks[row];
    }
    return nullptr;
}

int BlockListModel::rowCount(const QModelIndex& parent) const
{
    return static_cast<int>(mblocks.size());
}

QVariant BlockListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        int row = index.row();
        if (Qt::DisplayRole == role)
        {
            const MMBlock* block = mblocks[row];
            return QString("order=%1, alloc=%2, tags=%3, hash=%4")
                .arg(block->orderNo, 10)
                .arg(FormatNumberWithDigitGroups(block->allocByApp).c_str(), 12)
                .arg(block->tags, 8, 16)
                .arg(block->bktraceHash, 10);
        }
    }
    return QVariant();
}
