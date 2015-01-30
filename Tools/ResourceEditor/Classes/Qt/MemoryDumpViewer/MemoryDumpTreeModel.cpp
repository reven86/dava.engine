#include "MemoryDumpTreeItem.h"
#include "MemoryDumpTreeModel.h"
#include <QDebug>
#include <QStringList>

MemoryDumpTreeModel::MemoryDumpTreeModel(const DAVA::FilePath & filePath, QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Address" << "Memory(MB)" << "Memory(B)";
    rootItem = new MemoryDumpTreeItem(rootData);
//    setupModelData(object, rootItem);
    
    DAVA::File * file = DAVA::File::Create(filePath, DAVA::File::OPEN | DAVA::File::READ);
    
    DAVA::uint32 symbolsCount = 0;
    file->Read(&symbolsCount, 4);
    for (DAVA::uint32 k = 0; k < symbolsCount; ++k)
    {
        DAVA::uint64 ptr;
        file->Read(&ptr, 8);
        DAVA::char8 buffer[4096];
        file->ReadString(buffer, 4096);
        
        symbols[ptr] = QString(buffer);
    }
    
    setupModelData(file, rootItem);
    SafeRelease(file);
}

MemoryDumpTreeModel::~MemoryDumpTreeModel()
{
    delete rootItem;
}

int MemoryDumpTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<MemoryDumpTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}


QVariant MemoryDumpTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    MemoryDumpTreeItem *item = static_cast<MemoryDumpTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags MemoryDumpTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant MemoryDumpTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex MemoryDumpTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    MemoryDumpTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<MemoryDumpTreeItem*>(parent.internalPointer());

    MemoryDumpTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex MemoryDumpTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    MemoryDumpTreeItem *childItem = static_cast<MemoryDumpTreeItem*>(index.internalPointer());
    MemoryDumpTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int MemoryDumpTreeModel::rowCount(const QModelIndex &parent) const
{
    MemoryDumpTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<MemoryDumpTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void MemoryDumpTreeModel::setupModelData(DAVA::File * file, MemoryDumpTreeItem *parent)
{
    DAVA::uint32 nodeHeader, dataSize, childrenCount;
    DAVA::uint64 pointer;
    
    file->Read(&nodeHeader, 4);
    file->Read(&pointer, 8);
    file->Read(&dataSize, 4);
    file->Read(&childrenCount, 4);
    
    QList<QVariant> columnData;
    
    QString memMB;
    memMB = DAVA::Format("%0.2f MB",((float)dataSize / 1024.0f / 1024.0f)).c_str();
    
    columnData << symbols[pointer] << memMB << dataSize;
    MemoryDumpTreeItem * currentItem = new MemoryDumpTreeItem(columnData, parent);
    parent->appendChild(currentItem);
    
    for (DAVA::uint32 k = 0; k < childrenCount; ++k)
    {
        setupModelData(file, currentItem);
    }
}

/*    QList<TreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    for (auto key: object.keys())
    {
        //qWarning() << key;

        QJsonValue value = object.value(key);
        if (value.isObject())
        {
            QJsonObject objData = value.toObject();
            QList<QVariant> columnData;
            columnData << key << objData.value("size").toInt();
            TreeItem * childItem = new TreeItem(columnData, parents.last());
            parents.last()->appendChild(childItem);

            QJsonValue children = objData.value("children");
            if (children.type() != QJsonValue::Undefined)
            {
                setupModelData(children.toObject(), childItem);
            }
        }
    }*/

//    while (number < lines.count())
//    {
//        int position = 0;
//        while (position < lines[number].length()) {
//            if (lines[number].mid(position, 1) != " ")
//                break;
//            position++;
//        }

//        QString lineData = lines[number].mid(position).trimmed();

//        if (!lineData.isEmpty()) {
//            // Read the column data from the rest of the line.
//            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
//            QList<QVariant> columnData;
//            for (int column = 0; column < columnStrings.count(); ++column)
//                columnData << columnStrings[column];

//            if (position > indentations.last()) {
//                // The last child of the current parent is now the new parent
//                // unless the current parent has no children.

//                if (parents.last()->childCount() > 0) {
//                    parents << parents.last()->child(parents.last()->childCount()-1);
//                    indentations << position;
//                }
//            } else {
//                while (position < indentations.last() && parents.count() > 0) {
//                    parents.pop_back();
//                    indentations.pop_back();
//                }
//            }

//            // Append a new item to the current parent's list of children.
//            parents.last()->appendChild(new TreeItem(columnData, parents.last()));
//        }

//        ++number;
//    }
// }
