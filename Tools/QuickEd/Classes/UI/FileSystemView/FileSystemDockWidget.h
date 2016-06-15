#ifndef __QUICKED_FILE_SYSTEM_DIALOG_H__
#define __QUICKED_FILE_SYSTEM_DIALOG_H__

#include <QDockWidget>
#include <QModelIndex>
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

    void SetProjectDir(const QString& path);

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
    void OnCustomContextMenuRequested(const QPoint& pos);
    void OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    void RefreshActions();
    bool CanRemove(const QModelIndex& index) const;
    QString GetPathByCurrentPos();

    std::unique_ptr<Ui::FileSystemDockWidget> ui;
    QFileSystemModel* model = nullptr;
    QAction* newFolderAction = nullptr;
    QAction* newFileAction = nullptr;
    QAction* deleteAction = nullptr;
    QAction* showInSystemExplorerAction = nullptr;
    QAction* renameAction = nullptr;
    QAction* openFileAction = nullptr;
    QPoint menuInvokePos = QPoint(-1, -1);
};

#endif // __QUICKED_FILE_SYSTEM_DIALOG_H__
