#pragma once

#include <QAbstractListModel>

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

class BlockListModel : public QAbstractListModel
{
public:
    BlockListModel(QObject* parent = nullptr);
    virtual ~BlockListModel();

    void PrepareModel(DAVA::Vector<DAVA::MMBlock>&& blocks);
    void ResetModel();

    const DAVA::MMBlock* GetBlock(const QModelIndex& index) const;

    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    DAVA::Vector<DAVA::MMBlock> mblocks;
};
