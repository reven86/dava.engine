#ifndef __DIFFTREEMODEL_H__
#define __DIFFTREEMODEL_H__

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"

#include "GenericTreeModel.h"
#include "../MemoryDumpSession.h"

class DiffTreeModel : public QAbstractItemModel
{
public:
/*    BranchTreeModel(const MemoryDumpSession& session, QObject* parent = nullptr);
    virtual ~BranchTreeModel();

    void PrepareModel(const char* fromName);

    QVariant data(const QModelIndex& index, int role) const override;

    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    void BuildTree(const char* fromName);

private:
    const MemoryDumpSession& dumpSession;
    Branch* rootBranch;*/
};

#endif  // __DIFFTREEMODEL_H__
