#ifndef __BRANCHTREEMODEL_H__
#define __BRANCHTREEMODEL_H__

#include "Base/BaseTypes.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

struct Branch;
class MemorySnapshot;

class BranchFilterModel : public QSortFilterProxyModel
{
public:
    void SetFilter(DAVA::uint32 pools_, DAVA::uint32 tags_)
    {
        pools = pools_;
        tags = tags_;
        invalidateFilter();
    }

protected:
    //bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    DAVA::uint32 pools = 0;
    DAVA::uint32 tags = 0;
};

class BranchTreeModel : public QAbstractItemModel
{
public:
    enum {
        CLM_NAME = 0,
        CLM_STAT,
        NCOLUMNS = 2
    };

public:
    BranchTreeModel(const MemorySnapshot* snapshot, QObject* parent = nullptr);
    virtual ~BranchTreeModel();

    void PrepareModel(const DAVA::Vector<const DAVA::String*>& names);
    void ResetModel();

    QVariant data(const QModelIndex& index, int role) const override;

    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    const MemorySnapshot* snapshot = nullptr;
    Branch* rootBranch = nullptr;
};

#endif  // __BRANCHTREEMODEL_H__
