#pragma once

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"

#include "GenericTreeModel.h"
#include "../MemoryDumpSession.h"

class BlockListModel : public QAbstractItemModel
{
public:
    BlockListModel(bool diff, QObject* parent = nullptr);
    virtual ~BlockListModel();

    void PrepareModel(DAVA::Vector<const DAVA::MMBlock*>& v1);
    void PrepareDiffModel(DAVA::Vector<const DAVA::MMBlock*>& v1, DAVA::Vector<const DAVA::MMBlock*>& v2);

    const DAVA::MMBlock* GetBlock(const QModelIndex& index) const;

    QVariant data(const QModelIndex& index, int role) const override;

    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:

private:
    DAVA::Vector<const DAVA::MMBlock*> v;
    bool diffModel;
    int ncolumns;
};
