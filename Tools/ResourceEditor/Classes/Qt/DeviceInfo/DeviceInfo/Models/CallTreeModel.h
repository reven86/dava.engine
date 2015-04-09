#ifndef __CALLTREEMODEL_H__
#define __CALLTREEMODEL_H__

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"

#include "../DumpSession.h"

class CallTreeModel : public QAbstractItemModel
{
public:
    CallTreeModel(const char* filename, bool back, QObject* parent = nullptr);
    CallTreeModel(const char* filename, QObject* parent = nullptr);
    ~CallTreeModel();

    QVariant data(const QModelIndex& index, int role) const override;

    bool hasChildren(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

private:
    DumpSession dumpSession;
    DumpSession::Branch* rootBranch = nullptr;
};

#endif  // __CALLTREEMODEL_H__
