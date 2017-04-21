#pragma once

#include <Base/BaseTypes.h>

#include <QAbstractListModel>

class ProfilingSession;

class SnapshotListModel : public QAbstractListModel
{
public:
    SnapshotListModel(QObject* parent = nullptr);
    virtual ~SnapshotListModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void BeginNewProfileSession(ProfilingSession* profSession);
    void NewSnapshotArrived();

private:
    ProfilingSession* profileSession = nullptr;
};
