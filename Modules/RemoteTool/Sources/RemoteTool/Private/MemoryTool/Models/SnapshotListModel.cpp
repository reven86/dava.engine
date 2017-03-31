#include "RemoteTool/Private/MemoryTool/Models/SnapshotListModel.h"
#include "RemoteTool/Private/MemoryTool/ProfilingSession.h"

using namespace DAVA;

SnapshotListModel::SnapshotListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

SnapshotListModel::~SnapshotListModel()
{
}

int SnapshotListModel::rowCount(const QModelIndex& parent) const
{
    return profileSession != nullptr ? static_cast<int>(profileSession->SnapshotCount())
                                       :
                                       0;
}

QVariant SnapshotListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::DisplayRole == role)
    {
        if (Qt::Horizontal == orientation)
        {
            return QVariant("Memory snapshots");
        }
        else
        {
            return QVariant(section + 1);
        }
    }
    return QVariant();
}

QVariant SnapshotListModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && profileSession != nullptr)
    {
        int row = index.row();
        const MemorySnapshot& snapshot = profileSession->Snapshot(row);
        if (Qt::DisplayRole == role)
        {
            String filename = snapshot.FileName().GetFilename();
            return QString("%1 - %2")
            .arg(snapshot.Timestamp() / 1000)
            .arg(filename.c_str());
        }
        else if (Qt::BackgroundRole == role)
        {
            // TODO: maybe colorize loaded snapshots
        }
    }
    return QVariant();
}

void SnapshotListModel::BeginNewProfileSession(ProfilingSession* profSession)
{
    beginResetModel();
    profileSession = profSession;
    endResetModel();
}

void SnapshotListModel::NewSnapshotArrived()
{
    beginResetModel();
    endResetModel();
}
