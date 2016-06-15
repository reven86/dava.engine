#include "Debug/DVAssert.h"
#include "Logger/Logger.h"

#include "FileSystemDockWidget.h"
#include "ValidatedTextInputDialog.h"
#include "FileSystemModel.h"

#include "ui_FileSystemDockWidget.h"
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QModelIndex>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDirIterator>

#include "QtTools/FileDialog/FileDialog.h"
#include "Project/Project.h"

FileSystemDockWidget::FileSystemDockWidget(QWidget* parent)
    : QDockWidget(parent)
    , ui(new Ui::FileSystemDockWidget())
    , model(new FileSystemModel(this))
{
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
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
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

    ui->treeView->addAction(newFolderAction);
    ui->treeView->addAction(newFileAction);
    ui->treeView->addAction(deleteAction);
    ui->treeView->addAction(showInSystemExplorerAction);
    ui->treeView->addAction(renameAction);
    ui->treeView->addAction(openFileAction);
    installEventFilter(this);
    RefreshActions();
}

FileSystemDockWidget::~FileSystemDockWidget() = default;

void FileSystemDockWidget::SetProjectDir(const QString& path)
{
    if (path.isEmpty())
    {
        ui->treeView->hideColumn(0);
    }
    else
    {
        QDir dir(path);
        auto index = model->setRootPath(dir.path() + Project::GetScreensRelativePath());
        ui->treeView->setRootIndex(index);
        ui->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
        ui->treeView->showColumn(0);
    }
}

//refresh actions by menu invoke pos
void FileSystemDockWidget::RefreshActions()
{
    bool canCreateFile = !ui->treeView->isColumnHidden(0);
    bool canCreateDir = !ui->treeView->isColumnHidden(0); //column is hidden if no open projects
    bool canShow = false;
    bool canRename = false;
    auto index = ui->treeView->indexAt(menuInvokePos);

    if (index.isValid())
    {
        bool isDir = model->isDir(index);
        canCreateDir = isDir;
        canShow = true;
        canRename = true;
    }

    newFileAction->setEnabled(canCreateFile);
    newFolderAction->setEnabled(canCreateDir);
    showInSystemExplorerAction->setEnabled(canShow);
    showInSystemExplorerAction->setVisible(canShow);
    renameAction->setEnabled(canRename);
    renameAction->setVisible(canRename);
}

bool FileSystemDockWidget::CanRemove(const QModelIndex& index) const
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

QString FileSystemDockWidget::GetPathByCurrentPos()
{
    QModelIndex index = ui->treeView->indexAt(menuInvokePos);
    QString path;
    if (!index.isValid())
    {
        path = model->rootPath();
    }
    else
    {
        path = model->filePath(index);
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

    auto path = GetPathByCurrentPos();
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
    auto path = GetPathByCurrentPos();
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
    auto pathIn = GetPathByCurrentPos();
#ifdef Q_OS_MAC
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + pathIn + "\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#endif
#ifdef Q_OS_WIN
    QString param;
    param = QLatin1String("/select,");
    param += QDir::toNativeSeparators(pathIn);
    QString command = QString("explorer") + " " + param;
    QProcess::startDetached(command);
#endif
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

void FileSystemDockWidget::OnCustomContextMenuRequested(const QPoint& pos)
{
    menuInvokePos = pos;
    RefreshActions();
    QMenu::exec(ui->treeView->actions(), ui->treeView->viewport()->mapToGlobal(pos));
    menuInvokePos = QPoint(-1, -1);
}

void FileSystemDockWidget::OnSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    const auto& indexes = ui->treeView->selectionModel()->selectedIndexes();
    bool canRemove = !indexes.isEmpty();
    bool canOpen = false;
    for (auto index : indexes)
    {
        canRemove &= CanRemove(index);
        canOpen |= !model->isDir(index);
    }
    deleteAction->setEnabled(canRemove);
    openFileAction->setEnabled(canOpen);
    openFileAction->setVisible(canOpen);
}
