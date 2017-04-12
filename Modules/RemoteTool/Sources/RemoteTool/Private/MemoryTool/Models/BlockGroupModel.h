#pragma once

#include "RemoteTool/Private/MemoryTool/BlockGroup.h"

#include <Base/BaseTypes.h>

#include <QAbstractTableModel>

class BlockGroupModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum
    {
        ROLE_GROUP_POINTER = Qt::UserRole + 1
    };

public:
    BlockGroupModel(QObject* parent = nullptr);
    virtual ~BlockGroupModel();

    void SetBlockGroups(const DAVA::Vector<BlockGroup>* groups);

    // reimplemented QAbstractTableModel methods
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    const DAVA::Vector<BlockGroup>* groups = nullptr;
};
