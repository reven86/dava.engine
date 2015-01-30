#ifndef __QT_MEMORY_DUMP_TREE_MODEL_H__
#define __QT_MEMORY_DUMP_TREE_MODEL_H__

#include "DAVAEngine.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>

class MemoryDumpTreeItem;

class MemoryDumpTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    MemoryDumpTreeModel(const DAVA::FilePath & pathToLog, QObject *parent = 0);
    virtual ~MemoryDumpTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    void setupModelData(DAVA::File * file, MemoryDumpTreeItem *parent);

    std::map<DAVA::uint64, QString> symbols;
    MemoryDumpTreeItem *rootItem;
};

#endif // __QT_MEMORY_DUMP_TREE_MODEL_H__
