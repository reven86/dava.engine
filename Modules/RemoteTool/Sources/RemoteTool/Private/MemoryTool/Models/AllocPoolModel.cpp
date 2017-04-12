#include "RemoteTool/Private/MemoryTool/Models/AllocPoolModel.h"
#include "RemoteTool/Private/MemoryTool/Models/DataFormat.h"
#include "RemoteTool/Private/MemoryTool/ProfilingSession.h"

using namespace DAVA;

AllocPoolModel::AllocPoolModel(QObject* parent)
    : QAbstractTableModel(parent)
    , profileSession(nullptr)
    , timestamp(0)
{
}

AllocPoolModel::~AllocPoolModel()
{
}

int AllocPoolModel::rowCount(const QModelIndex& parent) const
{
    return profileSession != nullptr ? static_cast<int>(profileSession->AllocPoolCount())
                                       :
                                       0;
}

int AllocPoolModel::columnCount(const QModelIndex& parent) const
{
    return NCOLUMNS;
}

QVariant AllocPoolModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role)
    {
        if (Qt::Horizontal == orientation)
        {
            static const char* headers[NCOLUMNS] = {
                "Allocation pool",
                "Allocation size",
                "Overhead",
                "Block count"
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

QVariant AllocPoolModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && profileSession != nullptr)
    {
        int row = index.row();
        int clm = index.column();
        if (Qt::DisplayRole == role)
        {
            const AllocPoolStat& stat = curValues[row];
            switch (clm)
            {
            case CLM_NAME:
                return QVariant(profileSession->AllocPoolName(row).c_str());
            case CLM_ALLOC_APP:
                return FormatNumberWithDigitGroups(stat.allocByApp).c_str();
            case CLM_ALLOC_TOTAL:
                return FormatNumberWithDigitGroups(stat.allocTotal).c_str();
            case CLM_NBLOCKS:
                return FormatNumberWithDigitGroups(stat.blockCount).c_str();
            default:
                break;
            }
        }
        else if (Qt::BackgroundRole == role)
        {
            if (!poolColors.empty())
            {
                size_t index = static_cast<size_t>(row) % poolColors.size();
                return QVariant(poolColors[index]);
            }
        }
        else if (Qt::ForegroundRole == role)
        {
            if (!poolColors.empty())
                return QColor(Qt::white);
        }
    }
    return QVariant();
}

void AllocPoolModel::BeginNewProfileSession(ProfilingSession* profSession)
{
    beginResetModel();
    profileSession = profSession;
    curValues.clear();
    curValues.resize(profileSession->AllocPoolCount());
    endResetModel();
}

void AllocPoolModel::SetCurrentValues(const MemoryStatItem& item)
{
    Q_ASSERT(curValues.size() == item.PoolStat().size());
    timestamp = item.Timestamp();
    std::copy(item.PoolStat().begin(), item.PoolStat().end(), curValues.begin());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void AllocPoolModel::SetPoolColors(const DAVA::Vector<QColor>& colors)
{
    poolColors = colors;
}
