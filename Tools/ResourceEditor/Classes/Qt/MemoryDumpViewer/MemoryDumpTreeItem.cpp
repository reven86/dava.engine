#include <QStringList>
#include "MemoryDumpTreeItem.h"

MemoryDumpTreeItem::MemoryDumpTreeItem(const QList<QVariant> &data, MemoryDumpTreeItem *parent)
{
    m_parentItem = parent;
    m_itemData = data;
}

MemoryDumpTreeItem::~MemoryDumpTreeItem()
{
    qDeleteAll(m_childItems);
}

void MemoryDumpTreeItem::appendChild(MemoryDumpTreeItem *item)
{
    m_childItems.append(item);
}

MemoryDumpTreeItem *MemoryDumpTreeItem::child(int row)
{
    return m_childItems.value(row);
}

int MemoryDumpTreeItem::childCount() const
{
    return m_childItems.count();
}

int MemoryDumpTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant MemoryDumpTreeItem::data(int column) const
{
    return m_itemData.value(column);
}

MemoryDumpTreeItem *MemoryDumpTreeItem::parentItem()
{
    return m_parentItem;
}

int MemoryDumpTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<MemoryDumpTreeItem*>(this));

    return 0;
}