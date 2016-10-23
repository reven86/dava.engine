#pragma once

#include <QAbstractItemModel>
#include <QDir>

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
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
    void fetchMore(const QModelIndex& parent) override;
    bool canFetchMore(const QModelIndex& parent) const override;

    QModelIndex setRootPath(int subModelIndex, const QString& path);
    QString rootPath(int subModelIndex) const;

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
    void source_layoutChanged(const QList<QPersistentModelIndex>& parents = QList<QPersistentModelIndex>(), QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint);
    void source_layoutAboutToBeChanged(const QList<QPersistentModelIndex>& parents = QList<QPersistentModelIndex>(), QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint);
    void source_rowsAboutToBeInserted(QModelIndex p, int from, int to);
    void source_rowsInserted(QModelIndex p, int, int);
    void source_rowsAboutToBeRemoved(QModelIndex, int, int);
    void source_rowsRemoved(QModelIndex, int, int);
    void source_modelReset();
    void source_directoryLoaded(const QString& path);
    void source_rootPathChanged(const QString& newPath);
    void source_fileRenamed(const QString& path, const QString& oldName, const QString& newName);

private:
    QModelIndex mapToSource(const QModelIndex& index) const;
    QModelIndex mapFromSource(const QModelIndex& index) const;
    QFileSystemModel* mapSourceToModel(const QModelIndex& index) const;

    QDir::Filters filters;
    bool readOnlyEnable = false;
    bool nameFilterEnabled = false;
    QStringList localNameFilters;

    QVector<QFileSystemModel*> fileSystemModels;
    QVector<QModelIndex> fileSystemRootIndex;

    mutable QMap<QModelIndex, QModelIndex> mappingToSource;
    mutable QMap<QModelIndex, QModelIndex> mappingFromSource;
};