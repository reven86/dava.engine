#pragma once

#include <QAbstractItemModel>
#include <QDir>
#include <memory>

class QFileSystemModel;

class MultipleFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_SIGNALS :
    void
    rootPathChanged(const QString& newPath);
    void fileRenamed(const QString& path, const QString& oldName, const QString& newName);
    void directoryLoaded(const QString& path);

public:
    MultipleFileSystemModel(QObject* parent = nullptr);
    ~MultipleFileSystemModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    QModelIndex sibling(int row, int column, const QModelIndex& idx) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QMap<int, QVariant> itemData(const QModelIndex& index) const override;
    bool setItemData(const QModelIndex& index, const QMap<int, QVariant>& roles) override;

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    Qt::DropActions supportedDropActions() const override;

    void fetchMore(const QModelIndex& parent) override;
    bool canFetchMore(const QModelIndex& parent) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QModelIndex addPath(const QString& path, const QString& alias);

    void removePath(const QString& path);
    void removeAllPaths();

    QModelIndex index(const QString& path, int column = 0) const;

    void setFilter(QDir::Filters aFilters);
    QDir::Filters filter() const;

    void setReadOnly(bool enable);
    bool isReadOnly() const;

    void setNameFilterDisables(bool enable);
    bool nameFilterDisables() const;

    void setNameFilters(const QStringList& filters);
    QStringList nameFilters() const;

    QModelIndex mkdir(const QModelIndex& parent, const QString& name);

    QString fileName(const QModelIndex& index) const;
    bool remove(const QModelIndex& index);

    QString filePath(const QModelIndex& index) const;

    bool isDir(const QModelIndex& index) const;

private slots:
    void source_dataChanged(const QModelIndex& topleft, const QModelIndex& bottomright, const QVector<int>& roles);
    void source_rowsAboutToBeInserted(QModelIndex p, int from, int to);
    void source_rowsInserted(QModelIndex p, int, int);
    void source_rowsAboutToBeRemoved(QModelIndex, int, int);
    void source_rowsRemoved(QModelIndex, int, int);

private:
    struct FileSystemInfo
    {
        QString path;
        QString alias;
        QFileSystemModel* model = nullptr;
        QModelIndex root;
        //mutable QMap<QModelIndex, QModelIndex> mappingToSource;
        //mutable QMap<QModelIndex, QModelIndex> mappingFromSource;
    };

    void ConnectToModel(QFileSystemModel* model);
    void DisconnectFromModel(QFileSystemModel* model);

    bool IsProxyRoot(const QModelIndex& parent) const;

    const QModelIndex& MapToSource(const QModelIndex& proxyIndex) const;
    const QModelIndex& MapFromSource(const QModelIndex& sourceIndex) const;

    QFileSystemModel* GetModel(const QModelIndex& sourceIndex) const;
    const QModelIndex& GetRoot(const QModelIndex& sourceIndex) const;
    int GetModelIndex(const QModelIndex& sourceIndex) const;

    const FileSystemInfo& GetInfo(const QModelIndex& sourceIndex) const;

    QFileSystemModel* GetModel(int index) const
    {
        return fileSystemModels[index].model;
    }
    const QModelIndex& GetRoot(int index) const
    {
        return fileSystemModels[index].root;
    }
    const QString& GetAlias(int index) const
    {
        return fileSystemModels[index].alias;
    }

    QDir::Filters filters;
    bool readOnlyFlag = false;
    bool nameFilterFlag = false;
    QStringList nameFiltersList;

    QVector<FileSystemInfo> fileSystemModels;

    mutable QMap<QModelIndex, QModelIndex> mappingToSource;
    mutable QMap<QModelIndex, QModelIndex> mappingFromSource;
};