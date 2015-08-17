#ifndef __BRANCHDIFFTREEMODEL_H__
#define __BRANCHDIFFTREEMODEL_H__

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"

struct Branch;
struct BranchDiff;
class MemorySnapshot;

class BranchDiffTreeModel : public QAbstractItemModel
{
public:
    enum {
        CLM_NAME = 0,
        CLM_STAT1,
        CLM_STAT2,
        NCOLUMNS = 3
    };

public:
    BranchDiffTreeModel(const MemorySnapshot* snapshot1, const MemorySnapshot* snapshot2, QObject* parent = nullptr);
    virtual ~BranchDiffTreeModel();

    void PrepareModel(const DAVA::Vector<const DAVA::String*>& names);
    void ResetModel();

    QVariant data(const QModelIndex& index, int role) const override;

    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    const MemorySnapshot* snapshot1;
    const MemorySnapshot* snapshot2;
    BranchDiff* rootDiff = nullptr;
    Branch* rootLeft = nullptr;
    Branch* rootRight = nullptr;
};

#endif  // __BRANCHDIFFTREEMODEL_H__
