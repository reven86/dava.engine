#include "RemoteTool/Private/MemoryTool/Models/TagModel.h"
#include "RemoteTool/Private/MemoryTool/Models/DataFormat.h"
#include "RemoteTool/Private/MemoryTool/ProfilingSession.h"

using namespace DAVA;

TagModel::TagModel(QObject* parent)
    : QAbstractTableModel(parent)
    , profileSession(nullptr)
    , timestamp(0)
    , activeTags(0)
{
}

TagModel::~TagModel()
{
}

int TagModel::rowCount(const QModelIndex& parent) const
{
    return profileSession != nullptr ? static_cast<int>(profileSession->TagCount())
                                       :
                                       0;
}

int TagModel::columnCount(const QModelIndex& parent) const
{
    return NCOLUMNS;
}

QVariant TagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role)
    {
        if (Qt::Horizontal == orientation)
        {
            static const char* headers[NCOLUMNS] = {
                "Tag",
                "Allocation size",
                "Block count",
            };
            return QVariant(headers[section]);
        }
        else
        {
            return QVariant(section + 1);
        }
    }
    return QVariant();
}

QVariant TagModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && profileSession != nullptr)
    {
        int row = index.row();
        int clm = index.column();
        if (Qt::DisplayRole == role)
        {
            const TagAllocStat& stat = curValues[row];
            switch (clm)
            {
            case CLM_NAME:
                return QVariant(profileSession->TagName(row).c_str());
            case CLM_ALLOC_APP:
                return FormatNumberWithDigitGroups(stat.allocByApp).c_str();
            case CLM_NBLOCKS:
                return FormatNumberWithDigitGroups(stat.blockCount).c_str();
            default:
                break;
            }
        }
        else if (Qt::BackgroundRole == role)
        {
            uint32 mask = 1 << row;
            size_t colorIndex = activeTags & mask ? 1 : 0;
            return QVariant(colors[colorIndex]);
        }
    }
    return QVariant();
}

void TagModel::BeginNewProfileSession(ProfilingSession* profSession)
{
    beginResetModel();
    profileSession = profSession;
    curValues.clear();
    curValues.resize(profileSession->TagCount());
    endResetModel();
}

void TagModel::SetCurrentValues(const MemoryStatItem& item)
{
    Q_ASSERT(curValues.size() == item.TagStat().size());
    timestamp = item.Timestamp();
    activeTags = item.GeneralStat().activeTags;
    std::copy(item.TagStat().begin(), item.TagStat().end(), curValues.begin());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void TagModel::SetTagColors(QColor colorActive, QColor colorInactive)
{
    colors[0] = colorInactive;
    colors[1] = colorActive;
}
