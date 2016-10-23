#include "FileSystemDockWidget.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"


#include "ValidatedTextInputDialog.h"
#include "UI/FileSystemView/MultipleFileSystemModel.h"
#include "FileSystemModel.h"
#include "QtTools/FileDialogs/FileDialog.h"
#include "QtTools/Utils/Utils.h"
#include "QtTools/ProjectInformation/ProjectStructure.h"

#include "Project/Project.h"
#include "QtTools/FileDialogs/FindFileDialog.h"

#include "ui_FileSystemDockWidget.h"
#include <QClipboard>
#include <QMimeData>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QModelIndex>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDirIterator>
#include <QTimer>

namespace FileSystemDockWidgetDetails
{
template <typename T>
QModelIndexList CollectParentIndexes(const QModelIndex& index, const QModelIndex& rootIndex, T&& predicate)
{
    QModelIndexList modelIndexList;
    QModelIndex parentIndex = index.parent();

    while (parentIndex != rootIndex)
    {
        if (predicate(parentIndex))
        {
            //to expand or fetch items we reverse list to walk from parent to child
            modelIndexList.push_front(parentIndex);
        }
        parentIndex = parentIndex.parent();
    }
    return modelIndexList;
}
}

FileSystemDockWidget::FileSystemDockWidget(QWidget* parent)
    : QDockWidget(parent)
    , ui(new Ui::FileSystemDockWidget())
    //, model(new FileSystemModel(this))
    , model(new MultipleFileSystemModel(this))
{
    DAVA::Vector<DAVA::String> extensions = { "yaml" };
    projectStructure.reset(new ProjectStructure(extensions));

    ui->setupUi(this);
    ui->treeView->installEventFilter(this);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QWidget::customContextMenuRequested, this, &FileSystemDockWidget::OnCustomContextMenuRequested);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    model->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    setFilterFixedString("");
    model->setNameFilterDisables(false);
    model->setReadOnly(false);

    connect(model, &MultipleFileSystemModel::directoryLoaded, this, &FileSystemDockWidget::OnDirectoryLoaded);

    ui->treeView->setModel(model);
    ui->treeView->hideColumn(0);
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileSystemDockWidget::OnSelectionChanged);

    connect(ui->treeView, &QTreeView::doubleClicked, this, &FileSystemDockWidget::onDoubleClicked);
    connect(ui->filterLine, &QLineEdit::textChanged, this, &FileSystemDockWidget::setFilterFixedString);

    newFolderAction = new QAction(tr("Create folder"), this);
    connect(newFolderAction, &QAction::triggered, this, &FileSystemDockWidget::onNewFolder);

    newFileAction = new QAction(tr("Create file"), this);
    connect(newFileAction, &QAction::triggered, this, &FileSystemDockWidget::onNewFile);
    deleteAction = new QAction(tr("Delete"), this);
#if defined Q_OS_WIN
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
#elif defined Q_OS_MAC
    deleteAction->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
#endif // platform
    deleteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(deleteAction, &QAction::triggered, this, &FileSystemDockWidget::onDeleteFile);
    
#if defined Q_OS_WIN
    QString actionName = tr("Show in explorer");
#elif defined Q_OS_MAC
    QString actionName = tr("Show in finder");
#endif //Q_OS_WIN //Q_OS_MAC
    showInSystemExplorerAction = new QAction(actionName, this);
    connect(showInSystemExplorerAction, &QAction::triggered, this, &FileSystemDockWidget::OnShowInExplorer);

    renameAction = new QAction(tr("Rename"), this);
    connect(renameAction, &QAction::triggered, this, &FileSystemDockWidget::OnRename);

    openFileAction = new QAction(tr("Open File"), this);
    openFileAction->setShortcuts({ QKeySequence(Qt::Key_Return), QKeySequence(Qt::Key_Enter) });
    openFileAction->setShortcutContext(Qt::WidgetShortcut);
    connect(openFileAction, &QAction::triggered, this, &FileSystemDockWidget::OnOpenFile);

    copyInternalPathToFileAction = new QAction(tr("Copy Internal Path"), this);
    connect(copyInternalPathToFileAction, &QAction::triggered, this, &FileSystemDockWidget::OnCopyInternalPathToFile);

    findInFilesAction = FindFileDialog::CreateFindInFilesAction(parent);
    connect(findInFilesAction, &QAction::triggered, this, &FileSystemDockWidget::FindInFiles);
    addAction(findInFilesAction);

    ui->treeView->addAction(newFolderAction);
    ui->treeView->addAction(newFileAction);
    ui->treeView->addAction(deleteAction);
    ui->treeView->addAction(showInSystemExplorerAction);
    ui->treeView->addAction(renameAction);
    ui->treeView->addAction(openFileAction);
    ui->treeView->addAction(copyInternalPathToFileAction);
    installEventFilter(this);
    RefreshActions();
}

FileSystemDockWidget::~FileSystemDockWidget() = default;

void FileSystemDockWidget::SetProjectDir(const QString& path)
{
    isAvailable = !path.isEmpty();
    findInFilesAction->setEnabled(isAvailable);

    if (isAvailable)
    {
        QDir dir(path);
        QString uiPath = dir.path() + Project::GetScreensRelativePath();

        auto index = model->setRootPath(0, uiPath);
        model->setRootPath(1, uiPath);
        ui->treeView->setRootIndex(QModelIndex());
        ui->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
        ui->treeView->showColumn(0);

        projectStructure->SetProjectDirectory(uiPath.toStdString());
    }
    else
    {
        ui->treeView->hideColumn(0);

        projectStructure->SetProjectDirectory(DAVA::FilePath());
    }
}

void FileSystemDockWidget::FindInFiles()
{
    QString filePath = FindFileDialog::GetFilePath(projectStructure.get(), "yaml", parentWidget());
    if (filePath.isEmpty())
    {
        return;
    }
    ShowAndSelectFile(filePath);
    emit OpenPackageFile(filePath);
}

//refresh actions by menu invoke pos
void FileSystemDockWidget::RefreshActions()
{
    bool isProjectOpened = !ui->treeView->isColumnHidden(0); //column is hidden if no open projects
    bool canCreateFile = isProjectOpened;
    bool canCreateDir = isProjectOpened;
    bool canShow = false;
    bool canRename = false;
    bool canCopyInternalPath = false;
    const QModelIndex& index = ui->treeView->indexAt(menuInvokePos);

    if (index.isValid())
    {
        bool isDir = model->isDir(index);
        canCreateDir = isDir;
        canShow = true;
        canRename = true;
        canCopyInternalPath = true;
    }
    copyInternalPathToFileAction->setEnabled(canCopyInternalPath);
    UpdateActionsWithShortcutsState(QModelIndexList() << index);
    newFileAction->setEnabled(canCreateFile);
    newFolderAction->setEnabled(canCreateDir);
    showInSystemExplorerAction->setEnabled(canShow);
    showInSystemExplorerAction->setVisible(canShow);
    renameAction->setEnabled(canRename);
    renameAction->setVisible(canRename);
}

bool FileSystemDockWidget::CanDelete(const QModelIndex& index) const
{
    if (!model->isDir(index))
    {
        return true;
    }
    QDir dir(model->filePath(index));
    QDirIterator dirIterator(dir, QDirIterator::Subdirectories);
    while (dirIterator.hasNext())
    {
        if (dirIterator.next().endsWith(FileSystemModel::GetYamlExtensionString()))
        {
            return false;
        }
    }
    return true;
}

QString FileSystemDockWidget::GetPathByCurrentPos(ePathType pathType)
{
    QModelIndex index = ui->treeView->indexAt(menuInvokePos);
    QString path;
    if (!index.isValid())
    {
        path = model->rootPath(0);
    }
    else
    {
        path = model->filePath(index);
        if (pathType == DirPath)
        {
            QFileInfo fileInfo(path);
            if (fileInfo.isFile())
            {
                path = fileInfo.absolutePath();
            }
        }
    }
    return path + "/";
}

void FileSystemDockWidget::onDoubleClicked(const QModelIndex& index)
{
    if (!model->isDir(index))
    {
        emit OpenPackageFile(model->filePath(index));
    }
}

void FileSystemDockWidget::setFilterFixedString(const QString& filterStr)
{
    QStringList filters;
    filters << QString("*%1*" + FileSystemModel::GetYamlExtensionString()).arg(filterStr);
    model->setNameFilters(filters);
}

void FileSystemDockWidget::onNewFolder()
{
    ValidatedTextInputDialog dialog(this);
    QString newFolderName = tr("New Folder");
    dialog.setWindowTitle(newFolderName);
    dialog.setInputMethodHints(Qt::ImhUrlCharactersOnly);
    dialog.setLabelText("Enter new folder name:");
    dialog.SetWarningMessage("This folder already exists");

    auto path = GetPathByCurrentPos(DirPath);
    auto validateFunction = [path](const QString& text) {
        return !QFileInfo::exists(path + text);
    };

    dialog.SetValidator(validateFunction);

    if (!validateFunction(newFolderName))
    {
        QString newFolderName_ = newFolderName + " (%1)";
        int i = 1;
        do
        {
            newFolderName = newFolderName_.arg(i++);
        } while (!validateFunction(newFolderName));
    }
    dialog.setTextValue(newFolderName);

    int ret = dialog.exec();
    QString folderName = dialog.textValue();
    if (ret == QDialog::Accepted)
    {
        DVASSERT(!folderName.isEmpty());
        QModelIndex index = ui->treeView->indexAt(menuInvokePos);
        if (!index.isValid())
        {
            index = ui->treeView->rootIndex();
        }
        model->mkdir(index, folderName);
    }
    RefreshActions();
}

void FileSystemDockWidget::onNewFile()
{
    auto path = GetPathByCurrentPos(DirPath);
    QString strFile = FileDialog::getSaveFileName(this, tr("Create new file"), path, "*" + FileSystemModel::GetYamlExtensionString());
    if (strFile.isEmpty())
    {
        return;
    }
    if (!strFile.endsWith(FileSystemModel::GetYamlExtensionString()))
    {
        strFile += FileSystemModel::GetYamlExtensionString();
    }

    QFile file(strFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QString title = tr("Can not create file");
        QMessageBox::warning(this, title, title + tr("\n%1").arg(strFile));
        DAVA::Logger::Error("%s", QString(title + ": %1").arg(strFile).toUtf8().data());
    }
    file.close();
    RefreshActions();
}

void FileSystemDockWidget::onDeleteFile()
{
    QModelIndex index;
    if (menuInvokePos.x() != -1 && menuInvokePos.y() != -1)
    {
        index = ui->treeView->indexAt(menuInvokePos);
    }
    else
    {
        const auto& indexes = ui->treeView->selectionModel()->selectedIndexes();
        DVASSERT(indexes.size() == 1);
        index = indexes.first();
    }
    DVASSERT(index.isValid());
    bool isDir = model->isDir(index);
    QString title = tr("Delete ") + (isDir ? "folder" : "file") + "?";
    QString text = tr("Delete ") + (isDir ? "folder" : "file") + " \"" + model->fileName(index) + "\"" + (isDir ? " and its content" : "") + "?";
    if (QMessageBox::Yes == QMessageBox::question(this, title, text, QMessageBox::Yes | QMessageBox::No))
    {
        if (!model->remove(index))
        {
            DAVA::Logger::Error("can not remove file %s", model->isDir(index) ? "folder" : "file", model->fileName(index).toUtf8().data());
        }
    }
    RefreshActions();
}

void FileSystemDockWidget::OnShowInExplorer()
{
    auto pathIn = GetPathByCurrentPos(AnyPath);
    ShowFileInExplorer(pathIn);
}

void FileSystemDockWidget::OnRename()
{
    auto index = ui->treeView->indexAt(menuInvokePos);
    ui->treeView->edit(index);
}

void FileSystemDockWidget::OnOpenFile()
{
    const auto& indexes = ui->treeView->selectionModel()->selectedIndexes();
    for (auto index : indexes)
    {
        if (!model->isDir(index))
        {
            emit OpenPackageFile(model->filePath(index));
        }
    }
}

void FileSystemDockWidget::OnCopyInternalPathToFile()
{
    const QModelIndexList& indexes = ui->treeView->selectionModel()->selectedIndexes();
    for (const QModelIndex& index : indexes)
    {
        DAVA::FilePath path = model->filePath(index).toStdString();

        QClipboard* clipboard = QApplication::clipboard();
        QMimeData* data = new QMimeData();
        data->setText(QString::fromStdString(path.GetFrameworkPath()));
        clipboard->setMimeData(data);
    }
}

void FileSystemDockWidget::OnCustomContextMenuRequested(const QPoint& pos)
{
    menuInvokePos = pos;
    RefreshActions();
    QMenu::exec(ui->treeView->actions(), ui->treeView->viewport()->mapToGlobal(pos));
    menuInvokePos = QPoint(-1, -1);
}

void FileSystemDockWidget::OnSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    const QModelIndexList& indexes = ui->treeView->selectionModel()->selectedIndexes();
    UpdateActionsWithShortcutsState(indexes);
}

void FileSystemDockWidget::OnDirectoryLoaded()
{
    if (!indexToSetCurrent.isValid())
    {
        return;
    }
    QModelIndex rootIndex = ui->treeView->rootIndex();
    auto predicate = [this](const QModelIndex& index) {
        return model->canFetchMore(index);
    };
    QModelIndexList indexes = FileSystemDockWidgetDetails::CollectParentIndexes(indexToSetCurrent, rootIndex, predicate);
    if (!indexes.isEmpty())
    {
        return;
    }

    auto dummyPredicate = [this](const QModelIndex& index) {
        return true;
    };
    indexes = FileSystemDockWidgetDetails::CollectParentIndexes(indexToSetCurrent, rootIndex, dummyPredicate);
    for (const QModelIndex& index : indexes)
    {
        ui->treeView->expand(index);
    }

    ui->treeView->setCurrentIndex(indexToSetCurrent);
    indexToSetCurrent = QPersistentModelIndex();
}

void FileSystemDockWidget::UpdateActionsWithShortcutsState(const QModelIndexList& indexes)
{
    bool canDelete = false;
    bool canOpen = false;
    for (auto index : indexes)
    {
        if (!index.isValid())
        {
            continue;
        }
        canDelete |= CanDelete(index);
        canOpen |= !model->isDir(index);
    }
    deleteAction->setEnabled(canDelete);
    openFileAction->setEnabled(canOpen);
    openFileAction->setVisible(canOpen);
}

void FileSystemDockWidget::ShowAndSelectFile(const QString& filePath)
{
    DVASSERT(!filePath.isEmpty());
    indexToSetCurrent = model->index(filePath);
    if (indexToSetCurrent.isValid())
    {
        auto predicate = [this](const QModelIndex& index) -> bool {
            return model->canFetchMore(index);
        };
        //get unfetched indexes
        QModelIndexList indexes = FileSystemDockWidgetDetails::CollectParentIndexes(indexToSetCurrent, ui->treeView->rootIndex(), predicate);
        for (const QModelIndex& index : indexes)
        {
            model->fetchMore(index);
        }
        //nothing to fetch - can show selected file
        if (indexes.isEmpty())
        {
            OnDirectoryLoaded();
        }
    }
}
