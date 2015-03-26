#ifndef __BRANCHTREEMODEL_H__
#define __BRANCHTREEMODEL_H__

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"

#include "GenericTreeModel.h"
#include "../MemoryDumpSession.h"

class BranchTreeModel : public QAbstractItemModel
{
public:
    BranchTreeModel(const MemoryDumpSession& session, bool diff, QObject* parent = nullptr);
    virtual ~BranchTreeModel();

    void PrepareModel(const char* fromName);
    void PrepareDiffModel(const char* fromName);
    void PrepareModel(const DAVA::Vector<const char*>& names);
    void PrepareDiffModel(const DAVA::Vector<const char*>& names);


    QModelIndex Select(const DAVA::MMBlock* block) const;
    DAVA::Vector<QModelIndex> Select2(const DAVA::MMBlock* block) const;

    QVariant data(const QModelIndex& index, int role) const override;

    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    void BuildTree(const char* fromName);
    void BuildTree(const DAVA::Vector<const char*>& names);
    QVariant dataBranch(Branch* branch, int row, int clm) const;
    QVariant dataDiff(Branch* branch, int row, int clm) const;
    QVariant colorDiff(Branch* branch, int row, int clm) const;

    QModelIndex Find(Branch* p, const QModelIndex& par) const;
    void Find(Branch* p, const QModelIndex& par, DAVA::Vector<QModelIndex>& v) const;

private:
    const MemoryDumpSession& dumpSession;
    Branch* rootBranch;
    bool diffModel;
    int ncolumns;
};

#endif  // __BRANCHTREEMODEL_H__
