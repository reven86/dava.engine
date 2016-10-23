#include "MultipleFileSystemModel.h"
#include "Debug/DVAssert.h"

#include <QFileSystemModel>
#include "FileSystemModel.h"

MultipleFileSystemModel::MultipleFileSystemModel(QObject* parent /*= nullptr*/)
    : fileSystemModels({ new FileSystemModel(parent), new QFileSystemModel(parent) })
    , fileSystemRootIndex({ QModelIndex(), QModelIndex() })
{
    for (auto& model : fileSystemModels)
    {
        connect(model, &QFileSystemModel::rootPathChanged, this, &MultipleFileSystemModel::source_rootPathChanged);
        connect(model, &QFileSystemModel::fileRenamed, this, &MultipleFileSystemModel::source_fileRenamed);
        connect(model, &QFileSystemModel::directoryLoaded, this, &MultipleFileSystemModel::source_directoryLoaded);

        connect(model, &QFileSystemModel::dataChanged, this, &MultipleFileSystemModel::source_dataChanged);
        connect(model, &QFileSystemModel::layoutChanged, this, &MultipleFileSystemModel::source_layoutChanged);
        connect(model, &QFileSystemModel::layoutAboutToBeChanged, this, &MultipleFileSystemModel::source_layoutAboutToBeChanged);
        connect(model, &QFileSystemModel::rowsAboutToBeInserted, this, &MultipleFileSystemModel::source_rowsAboutToBeInserted);
        connect(model, &QFileSystemModel::rowsInserted, this, &MultipleFileSystemModel::source_rowsInserted);
        connect(model, &QFileSystemModel::rowsAboutToBeRemoved, this, &MultipleFileSystemModel::source_rowsAboutToBeRemoved);
        connect(model, &QFileSystemModel::rowsRemoved, this, &MultipleFileSystemModel::source_rowsRemoved);
        connect(model, &QFileSystemModel::modelReset, this, &MultipleFileSystemModel::source_modelReset);
    }
}

MultipleFileSystemModel::~MultipleFileSystemModel()
{
    for (auto& model : fileSystemModels)
    {
        delete model;
    }
}

QModelIndex MultipleFileSystemModel::setRootPath(int subModelIndex, const QString& path)
{
    fileSystemRootIndex[subModelIndex] = fileSystemModels[subModelIndex]->setRootPath(path);

    return createIndex(subModelIndex, 0, subModelIndex);
}

QString MultipleFileSystemModel::rootPath(int subModelIndex) const
{
    return fileSystemModels[subModelIndex]->rootPath();
}

void MultipleFileSystemModel::setFilter(QDir::Filters aFilters)
{
    filters = aFilters;
    for (auto& model : fileSystemModels)
    {
        model->setFilter(filters);
    }
}

QDir::Filters MultipleFileSystemModel::filter() const
{
    return filters;
}

void MultipleFileSystemModel::setReadOnly(bool enable)
{
    readOnlyEnable = enable;
    for (auto& model : fileSystemModels)
    {
        model->setReadOnly(readOnlyEnable);
    }
}

bool MultipleFileSystemModel::isReadOnly() const
{
    return readOnlyEnable;
}

void MultipleFileSystemModel::setNameFilterDisables(bool enable)
{
    nameFilterEnabled = enable;
    for (auto& model : fileSystemModels)
    {
        model->setNameFilterDisables(nameFilterEnabled);
    }
}

bool MultipleFileSystemModel::nameFilterDisables() const
{
    return nameFilterEnabled;
}

void MultipleFileSystemModel::setNameFilters(const QStringList& filters)
{
    localNameFilters = filters;
    for (auto& model : fileSystemModels)
    {
        model->setNameFilters(localNameFilters);
    }
}

QStringList MultipleFileSystemModel::nameFilters() const
{
    return localNameFilters;
}

QModelIndex MultipleFileSystemModel::mkdir(const QModelIndex& parent, const QString& name)
{
    auto innerParent = mapToSource(parent);
    return mapFromSource(mapSourceToModel(innerParent)->mkdir(innerParent, name));
}

QString MultipleFileSystemModel::fileName(const QModelIndex& index) const
{
    auto innerIndex = mapToSource(index);
    return mapSourceToModel(innerIndex)->fileName(innerIndex);
}

bool MultipleFileSystemModel::remove(const QModelIndex& index)
{
    auto innerIndex = mapToSource(index);
    return mapSourceToModel(innerIndex)->remove(innerIndex);
}

QString MultipleFileSystemModel::filePath(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QString();
    }
    if (!index.parent().isValid())
    {
        return "Fake_file";
    }

    auto innerIndex = mapToSource(index);
    return mapSourceToModel(innerIndex)->filePath(innerIndex);
}

bool MultipleFileSystemModel::isDir(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return false;
    }
    if (!index.parent().isValid())
    {
        return false;
    }

    auto innerIndex = mapToSource(index);
    return mapSourceToModel(innerIndex)->isDir(innerIndex);
}

QModelIndex MultipleFileSystemModel::index(const QString& path, int column /*= 0*/) const
{
    for (auto& model : fileSystemModels)
    {
        QModelIndex index = model->index(path, column);
        if (index.isValid())
        {
            return mapFromSource(index);
        }
    }

    return QModelIndex();
}

QModelIndex MultipleFileSystemModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    if (!parent.isValid())
    {
        return createIndex(row, column, row);
    }

    if (parent.internalId() < fileSystemModels.size())
    {
        QModelIndex sourceIndex = fileSystemModels[parent.row()]->index(row, column, fileSystemRootIndex[parent.row()]);
        QModelIndex index = createIndex(row, column, sourceIndex.internalPointer());
        mappingToSource[index] = sourceIndex;
        mappingFromSource[sourceIndex] = index;
        return index;
    }

    QModelIndex sourceParent = mapToSource(parent);
    QModelIndex sourceIndex = sourceParent.model()->index(row, column, sourceParent);
    QModelIndex index = createIndex(row, column, sourceIndex.internalPointer());
    mappingToSource[index] = sourceIndex;
    mappingFromSource[sourceIndex] = index;
    return index;
}

QModelIndex MultipleFileSystemModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
    {
        DVASSERT(false && "invalid child passed to parent function");
        return QModelIndex();
    }

    if (child.internalId() < fileSystemModels.size())
    {
        return QModelIndex();
    }

    auto sourceChild = mapToSource(child);
    auto sourceParent = sourceChild.model()->parent(sourceChild);

    auto it = std::find(fileSystemModels.begin(), fileSystemModels.end(), sourceChild.model());
    DVASSERT(it != fileSystemModels.end());
    auto index = std::distance(fileSystemModels.begin(), it);

    if (sourceParent == fileSystemRootIndex[index])
    {
        return createIndex(index, 0, index);
    }

    return mapFromSource(sourceParent);
}

int MultipleFileSystemModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        return fileSystemModels.size();
    }

    if (parent.internalId() < fileSystemModels.size())
    {
        return fileSystemModels[parent.row()]->rowCount(fileSystemRootIndex[parent.row()]);
    }

    auto sourceParent = mapToSource(parent);
    return sourceParent.model()->rowCount(sourceParent);
}

int MultipleFileSystemModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 1;
}

QVariant MultipleFileSystemModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.internalId() < fileSystemModels.size())
    {
        switch (role)
        {
        case Qt::DisplayRole:
            return QVariant("Fake Root");
        }

        return QVariant();
    }

    auto sourceIndex = mapToSource(index);
    return sourceIndex.model()->data(sourceIndex, role);
}

bool MultipleFileSystemModel::hasChildren(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!parent.isValid())
    {
        return true;
    }
    if (parent.internalId() < fileSystemModels.size())
    {
        return true;
    }

    auto sourceParent = mapToSource(parent);
    return sourceParent.model()->hasChildren(sourceParent);
}

void MultipleFileSystemModel::fetchMore(const QModelIndex& parent)
{
    if (!parent.isValid())
    {
        return;
    }
    if (parent.internalId() < fileSystemModels.size())
    {
        fileSystemModels[parent.row()]->fetchMore(fileSystemRootIndex[parent.row()]);
        return;
    }

    auto sourceParent = mapToSource(parent);
    mapSourceToModel(sourceParent)->fetchMore(sourceParent);
}

bool MultipleFileSystemModel::canFetchMore(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return false;
    }
    if (parent.internalId() < fileSystemModels.size())
    {
        return fileSystemModels[parent.row()]->canFetchMore(fileSystemRootIndex[parent.row()]);
    }

    auto sourceParent = mapToSource(parent);
    return sourceParent.model()->canFetchMore(sourceParent);
}

void MultipleFileSystemModel::source_dataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight, const QVector<int>& roles)
{
    const QModelIndex topLeft = mapFromSource(sourceTopLeft);
    const QModelIndex bottomRight = mapFromSource(sourceBottomRight);
    emit dataChanged(topLeft, bottomRight, roles);
}

void MultipleFileSystemModel::source_layoutChanged(const QList<QPersistentModelIndex>& parents /*= QList<QPersistentModelIndex>()*/, QAbstractItemModel::LayoutChangeHint hint /*= QAbstractItemModel::NoLayoutChangeHint*/)
{
    emit layoutChanged(parents, hint);
}

void MultipleFileSystemModel::source_layoutAboutToBeChanged(const QList<QPersistentModelIndex>& parents /*= QList<QPersistentModelIndex>()*/, QAbstractItemModel::LayoutChangeHint hint /*= QAbstractItemModel::NoLayoutChangeHint*/)
{
    emit layoutAboutToBeChanged(parents, hint);
}

void MultipleFileSystemModel::source_rowsAboutToBeInserted(QModelIndex p, int from, int to)
{
    //QModelIndex p_p = mapFromSource(p);
    //    emit rowsAboutToBeInserted(p_p, from, to);
}

void MultipleFileSystemModel::source_rowsInserted(QModelIndex p, int from, int to)
{
    //QModelIndex p_p = mapFromSource(p);
    //    emit rowsAboutToBeInserted(p_p, from, to);
}

void MultipleFileSystemModel::source_rowsAboutToBeRemoved(QModelIndex, int, int)
{
}

void MultipleFileSystemModel::source_rowsRemoved(QModelIndex, int, int)
{
}

void MultipleFileSystemModel::source_modelReset()
{
}

void MultipleFileSystemModel::source_directoryLoaded(const QString& path)
{
    emit directoryLoaded(path);
}

void MultipleFileSystemModel::source_rootPathChanged(const QString& newPath)
{
    emit rootPathChanged(newPath);
}

void MultipleFileSystemModel::source_fileRenamed(const QString& path, const QString& oldName, const QString& newName)
{
    emit fileRenamed(path, oldName, newName);
}

QModelIndex MultipleFileSystemModel::mapToSource(const QModelIndex& proxy) const
{
    if (!proxy.isValid())
    {
        DVASSERT(false);
        return QModelIndex();
    }

    if (proxy.internalId() < fileSystemModels.size())
    {
        return fileSystemRootIndex[proxy.internalId()];
    }

    auto it = mappingToSource.find(proxy);
    if (it != mappingToSource.end())
    {
        return it.value();
    }

    return QModelIndex();
}

QModelIndex MultipleFileSystemModel::mapFromSource(const QModelIndex& index) const
{
    DVASSERT(index.isValid());

    auto it = mappingFromSource.find(index);
    if (it != mappingFromSource.end())
    {
        return it.value();
    }
    return QModelIndex();
}

QFileSystemModel* MultipleFileSystemModel::mapSourceToModel(const QModelIndex& index) const
{
    for (auto& model : fileSystemModels)
    {
        if (model == index.model())
        {
            return model;
        }
    }

    return nullptr;
}
