#pragma once

#include <QAbstractItemModel>

#include "Base/BaseTypes.h"

#include "GenericTreeModel.h"
#include "../MemoryDumpSession.h"

class BlockListModel : public QAbstractItemModel
{
    struct DiffStruct
    {
        enum {
            D_SAME = 0,
            D_HASH,
            D_DIFF
        };
        using pair = std::pair<const DAVA::MMBlock*, const DAVA::MMBlock*>;
        using range = std::pair<int, int>;
        DAVA::Vector<pair> v[3];
        range r[3];
        int ntotal;

        int in_range(int k) const
        {
            for (int i = 0;i < 3;++i)
            {
                if (r[i].first <= k && k < r[i].second)
                {
                    return i;
                }
            }
            return -1;
        }

        //DAVA::Vector<const DAVA::MMBlock*> same;    // Same blocks on left and right
        //DAVA::Vector<const DAVA::MMBlock*> same2;   // Same blocks on left and right
        //DAVA::Vector<const DAVA::MMBlock*> left;    // Blocks only at left
        //DAVA::Vector<const DAVA::MMBlock*> right;   // Blocks only at right
    };

public:
    BlockListModel(bool diff, QObject* parent = nullptr);
    virtual ~BlockListModel();

    void PrepareModel(DAVA::Vector<const DAVA::MMBlock*>& v1);
    void PrepareDiffModel(bool cmpCallstack, DAVA::Vector<const DAVA::MMBlock*>& v1, DAVA::Vector<const DAVA::MMBlock*>& v2);

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

    DiffStruct diff;
};
