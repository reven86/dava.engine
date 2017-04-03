#pragma once

#include <Functional/Signal.h>

#include <QWidget>
#include <QModelIndex>
#include <memory>

class QLineEdit;
class QTreeView;
class QFileSystemModel;
class QItemSelection;

namespace DAVA
{
class Any;
namespace TArc
{
class ContextAccessor;
class FieldBinder;
}
}

class FileSystemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileSystemWidget(DAVA::TArc::ContextAccessor* accessor, QWidget* parent = nullptr);
    ~FileSystemWidget();

    void SelectFile(const QString& filePath);

    DAVA::Signal<const QString&> openFile;

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

    void InitUI();
    void BindFields();
    void SetResourceDirectory(const QString& path);

    void OnProjectPathChanged(const DAVA::Any& projectPath);

    enum ePathType
    {
        AnyPath,
        DirPath
    };
    QString GetPathByCurrentPos(ePathType pathType);

    QFileSystemModel* model = nullptr;
    QAction* newFolderAction = nullptr;
    QAction* newFileAction = nullptr;
    QAction* deleteAction = nullptr;
    QAction* showInSystemExplorerAction = nullptr;
    QAction* renameAction = nullptr;
    QAction* openFileAction = nullptr;
    QAction* copyInternalPathToFileAction = nullptr;

    QPoint menuInvokePos = QPoint(-1, -1);

    QLineEdit* filterLine = nullptr;
    QTreeView* treeView = nullptr;

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
