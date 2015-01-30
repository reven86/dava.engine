#ifndef __QT_MEMORY_DUMP_TREE_ITEM_H__
#define __QT_MEMORY_DUMP_TREE_ITEM_H__

#include <QList>
#include <QVariant>

class MemoryDumpTreeItem
{
public:
    explicit MemoryDumpTreeItem(const QList<QVariant> &data, MemoryDumpTreeItem *parentItem = 0);
    ~MemoryDumpTreeItem();

    void appendChild(MemoryDumpTreeItem *child);

    MemoryDumpTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    MemoryDumpTreeItem *parentItem();

private:
    QList<MemoryDumpTreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    MemoryDumpTreeItem *m_parentItem;
};


#endif // __QT_MEMORY_DUMP_TREE_ITEM_H__
