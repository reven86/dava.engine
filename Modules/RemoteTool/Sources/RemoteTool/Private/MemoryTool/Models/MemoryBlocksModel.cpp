#include "RemoteTool/Private/MemoryTool/Models/MemoryBlocksModel.h"
#include "RemoteTool/Private/MemoryTool/Models/DataFormat.h"
#include "RemoteTool/Private/MemoryTool/ProfilingSession.h"

#include <Debug/DVAssert.h>
#include <MemoryManager/MemoryManagerTypes.h>

using namespace DAVA;

MemoryBlocksModel::MemoryBlocksModel(const ProfilingSession* session_, QObject* parent)
    : QAbstractTableModel(parent)
    , session(session_)
{
    DVASSERT(session != nullptr);
}

MemoryBlocksModel::~MemoryBlocksModel() = default;

void MemoryBlocksModel::SetBlockLink(const BlockLink* blockLink_)
{
    beginResetModel();
    blockLink = blockLink_;
    endResetModel();
}

int MemoryBlocksModel::rowCount(const QModelIndex& parent) const
{
    return blockLink != nullptr ? static_cast<int>(blockLink->items.size()) : 0;
}

int MemoryBlocksModel::columnCount(const QModelIndex& parent) const
{
    return blockLink != nullptr ? static_cast<int>(blockLink->linkCount) : 0;
}

QVariant MemoryBlocksModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        const BlockLink::Item& item = blockLink->items[index.row()];
        if (Qt::DisplayRole == role)
        {
            const MMBlock* block = BlockLink::Block(item, index.column());
            if (block != nullptr)
            {
                const String& poolName = session->AllocPoolNameByMask(block->pool);
                return QString("order=%1;size=%2;pool=[%3];backtrace=%4;%5")
                .arg(block->orderNo)
                .arg(FormatNumberWithDigitGroups(block->allocByApp).c_str())
                .arg(poolName.c_str())
                .arg(block->bktraceHash)
                .arg(block->tags != 0 ? TagsToString(block->tags) : QString());
            }
        }
        else if (ROLE_LINKITEM_POINTER == role)
        {
            return QVariant::fromValue(const_cast<void*>(static_cast<const void*>(&item)));
        }
    }
    return QVariant();
}

QVariant MemoryBlocksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && Qt::Horizontal == orientation && blockLink != nullptr)
    {
        DVASSERT(section < static_cast<int>(blockLink->linkCount));
        String filename = blockLink->sourceSnapshots[section]->FileName().GetFilename();
        return QString("Memory blocks of %1").arg(filename.c_str());
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QString MemoryBlocksModel::TagsToString(uint32 tags) const
{
    QString result("tags=");
    int bit = static_cast<int>(HighestBitIndex(tags));
    for (; bit >= 0 && tags != 0; --bit)
    {
        tags &= ~(1 << bit);
        result += session->TagName(bit).c_str();
        result += ',';
    }
    result.chop(1); // Remove last comma
    return result;
}

//////////////////////////////////////////////////////////////////////////
MemoryBlocksFilterModel::MemoryBlocksFilterModel(MemoryBlocksModel* model, QObject* parent)
    : QSortFilterProxyModel(parent)
    , lessThanPredicate([](const BlockLink::Item& l, const BlockLink::Item& r) { return BlockLink::AnyBlock(l)->orderNo < BlockLink::AnyBlock(r)->orderNo; })
    , filterPredicate([](const BlockLink::Item&) { return true; })
{
    QSortFilterProxyModel::setDynamicSortFilter(false);
    QSortFilterProxyModel::setSourceModel(model);
}

MemoryBlocksFilterModel::~MemoryBlocksFilterModel() = default;

void MemoryBlocksFilterModel::ClearFilter()
{
    dontFilter = true;
    invalidateFilter();
}

QVariant MemoryBlocksFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && Qt::Vertical == orientation)
    {
        return QVariant(section + 1);
    }
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

bool MemoryBlocksFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QAbstractItemModel* source = sourceModel();
    QVariant vleft = source->data(left, MemoryBlocksModel::ROLE_LINKITEM_POINTER);
    QVariant vright = source->data(right, MemoryBlocksModel::ROLE_LINKITEM_POINTER);

    const BlockLink::Item* itemLeft = static_cast<const BlockLink::Item*>(vleft.value<void*>());
    const BlockLink::Item* itemRight = static_cast<const BlockLink::Item*>(vright.value<void*>());
    DVASSERT(itemLeft != nullptr && itemRight != nullptr);
    return lessThanPredicate(*itemLeft, *itemRight);
}

bool MemoryBlocksFilterModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex& /*source_parent*/) const
{
    return true;
}

bool MemoryBlocksFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (!dontFilter)
    {
        QAbstractItemModel* source = sourceModel();
        QModelIndex index = source->index(source_row, 0, source_parent);

        QVariant v = source->data(index, MemoryBlocksModel::ROLE_LINKITEM_POINTER);
        const BlockLink::Item* item = static_cast<const BlockLink::Item*>(v.value<void*>());
        ;
        DVASSERT(item != nullptr);
        return filterPredicate(*item);
    }
    return true;
}
