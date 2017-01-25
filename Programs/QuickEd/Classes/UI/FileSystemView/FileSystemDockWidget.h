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

class FileSystemDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit FileSystemDockWidget(QWidget* parent = nullptr);
    ~FileSystemDockWidget();

    void SetResourceDirectory(const QString& path);
    void SelectFile(const QString& filePath);

signals:
    void OpenPackageFile(const QString& path);

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

private:
    void RefreshActions();
    bool CanDelete(const QModelIndex& index) const;
    void UpdateActionsWithShortcutsState(const QModelIndexList& modelIndexes);

    enum ePathType
    {
        AnyPath,
        DirPath
    };
    QString GetPathByCurrentPos(ePathType pathType);

    std::unique_ptr<Ui::FileSystemDockWidget> ui;
    QFileSystemModel* model = nullptr;
    QAction* newFolderAction = nullptr;
    QAction* newFileAction = nullptr;
    QAction* deleteAction = nullptr;
    QAction* showInSystemExplorerAction = nullptr;
    QAction* renameAction = nullptr;
    QAction* openFileAction = nullptr;
    QAction* copyInternalPathToFileAction = nullptr;

    QPoint menuInvokePos = QPoint(-1, -1);
};
