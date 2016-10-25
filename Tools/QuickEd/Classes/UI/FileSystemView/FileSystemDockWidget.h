#pragma once

#include <QDockWidget>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <memory>

namespace Ui
{
class FileSystemDockWidget;
}

class QFileSystemModel;
class QInputDialog;
class QItemSelection;
class QMouseEvent;
class ProjectStructure;

class FileSystemDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit FileSystemDockWidget(QWidget* parent = nullptr);
    ~FileSystemDockWidget();

    void AddPath(const QString& path, const QString& displayName);
    void RemovePath(const QString& path);
    void RemoveAllPaths();

signals:
    void OpenPackageFile(const QString& path);

public slots:
    void FindInFiles();

private slots:
    void onDoubleClicked(const QModelIndex& index);
    void setFilterFixedString(const QString& filterStr);
    void onNewFolder();
    void onNewFile();
    void onDeleteFile();
    void OnShowInExplorer();
    void OnRename();
    void OnOpenFile();
    void OnCopyInternalPathToFile();
    void OnCustomContextMenuRequested(const QPoint& pos);
    void OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void OnDirectoryLoaded();

private:
    void RefreshActions();
    bool CanDelete(const QModelIndex& index) const;
    void UpdateActionsWithShortcutsState(const QModelIndexList& modelIndexes);

    void ShowAndSelectFile(const QString& filePath);

    enum ePathType
    {
        AnyPath,
        DirPath
    };
    QString GetPathByCurrentPos(ePathType pathType);

    std::unique_ptr<Ui::FileSystemDockWidget> ui;
    //    QFileSystemModel* model = nullptr;
    class MultipleFileSystemModel* model = nullptr;
    QAction* newFolderAction = nullptr;
    QAction* newFileAction = nullptr;
    QAction* deleteAction = nullptr;
    QAction* showInSystemExplorerAction = nullptr;
    QAction* renameAction = nullptr;
    QAction* openFileAction = nullptr;
    QAction* copyInternalPathToFileAction = nullptr;

    QAction* findInFilesAction = nullptr;

    QPoint menuInvokePos = QPoint(-1, -1);

    bool isAvailable = false;

    std::unique_ptr<ProjectStructure> projectStructure;
    //we set current index asyncronysly, when model emits signal "directoryLoaded"
    QPersistentModelIndex indexToSetCurrent;
};
