#include "RemoteTool/Private/MemoryTool/Models/BlockGroupModel.h"
#include "RemoteTool/Private/MemoryTool/Models/DataFormat.h"

using namespace DAVA;

BlockGroupModel::BlockGroupModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

BlockGroupModel::~BlockGroupModel() = default;

void BlockGroupModel::SetBlockGroups(const DAVA::Vector<BlockGroup>* groups)
{
    beginResetModel();
    this->groups = groups;
    endResetModel();
}

int BlockGroupModel::rowCount(const QModelIndex& parent) const
{
    return groups != nullptr ? static_cast<int>(groups->size()) : 0;
}

int BlockGroupModel::columnCount(const QModelIndex& parent) const
{
    if (groups != nullptr && !groups->empty())
    {
        return static_cast<int>(groups->operator[](0).blockLink.linkCount);
    }
    return 0;
}

QVariant BlockGroupModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        const BlockGroup& curGroup = groups->operator[](index.row());
        if (Qt::DisplayRole == role)
        {
            int column = index.column();
            QString s = QString("size=%1;nblocks=%2")
                        .arg(FormatNumberWithDigitGroups(curGroup.blockLink.allocSize[column]).c_str())
                        .arg(FormatNumberWithDigitGroups(curGroup.blockLink.blockCount[column]).c_str());
            if (0 == column)
            {
                return QString("%1;%2")
                .arg(curGroup.title.c_str())
                .arg(s);
            }
            else
                return s;
        }
        else if (ROLE_GROUP_POINTER == role)
        {
            return QVariant::fromValue(const_cast<void*>(static_cast<const void*>(&curGroup)));
        }
    }
    return QVariant();
}

QVariant BlockGroupModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && Qt::Horizontal == orientation)
    {
        return QString("Block grouping");
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}
