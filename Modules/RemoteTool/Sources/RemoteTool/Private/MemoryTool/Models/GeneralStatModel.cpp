#include "RemoteTool/Private/MemoryTool/Models/GeneralStatModel.h"
#include "RemoteTool/Private/MemoryTool/Models/DataFormat.h"
#include "RemoteTool/Private/MemoryTool/ProfilingSession.h"

using namespace DAVA;

GeneralStatModel::GeneralStatModel(QObject* parent)
    : QAbstractTableModel(parent)
    , profileSession(nullptr)
    , timestamp(0)
    , curValues()
{
}

GeneralStatModel::~GeneralStatModel()
{
}

int GeneralStatModel::rowCount(const QModelIndex& parent) const
{
    return NROWS;
}

int GeneralStatModel::columnCount(const QModelIndex& parent) const
{
    return NCOLUMNS;
}

QVariant GeneralStatModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role)
    {
        if (Qt::Horizontal == orientation)
        {
            static const char* headers[NCOLUMNS] = {
                "Value"
            };
            return QVariant(headers[section]);
        }
        else
        {
            static const char* headers[NROWS] = {
                "Internal allocation size",
                "Total internal allocation size",
                "Internal block count",
                "Ghost allocation size",
                "Ghost block count",
                "Total allocation count"
            };
            return QVariant(headers[section]);
        }
    }
    return QVariant();
}

QVariant GeneralStatModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && profileSession != nullptr)
    {
        int row = index.row();
        if (Qt::DisplayRole == role)
        {
            switch (row)
            {
            case ROW_ALLOC_INTERNAL:
                return FormatNumberWithDigitGroups(curValues.allocInternal).c_str();
            case ROW_ALLOC_INTERNAL_TOTAL:
                return FormatNumberWithDigitGroups(curValues.allocInternalTotal).c_str();
            case ROW_NBLOCKS_INTERNAL:
                return FormatNumberWithDigitGroups(curValues.internalBlockCount).c_str();
            case ROW_ALLOC_GHOST:
                return FormatNumberWithDigitGroups(curValues.ghostSize).c_str();
            case ROW_NBLOCKS_GHOST:
                return FormatNumberWithDigitGroups(curValues.ghostBlockCount).c_str();
            case ROW_TOTAL_ALLOC_COUNT:
                return FormatNumberWithDigitGroups(curValues.nextBlockNo).c_str();
            default:
                break;
            }
        }
    }
    return QVariant();
}

void GeneralStatModel::BeginNewProfileSession(ProfilingSession* profSession)
{
    beginResetModel();
    profileSession = profSession;
    curValues = GeneralAllocStat();
    endResetModel();
}

void GeneralStatModel::SetCurrentValues(const MemoryStatItem& item)
{
    timestamp = item.Timestamp();
    curValues = item.GeneralStat();
    emit dataChanged(QModelIndex(), QModelIndex());
}
