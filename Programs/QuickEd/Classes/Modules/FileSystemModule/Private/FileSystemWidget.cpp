#include "Modules/FileSystemModule/FileSystemWidget.h"
#include "Modules/FileSystemModule/FileSystemModel.h"
#include "Modules/FileSystemModule/Private/ValidatedTextInputDialog.h"

#include "Modules/LegacySupportModule/Private/Project.h"

#include <TArc/Core/FieldBinder.h>

#include <QtTools/FileDialogs/FileDialog.h>
#include <QtTools/Utils/Utils.h>
#include <QtHelpers/HelperFunctions.h>

#include <Debug/DVAssert.h>
#include <Logger/Logger.h>
#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>

#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTreeView>
#include <QVBoxLayout>

#include <QModelIndex>
#include <QMimeData>
#include <QClipboard>
#include <QDirIterator>
#include <QApplication>

FileSystemWidget::FileSystemWidget(DAVA::TArc::ContextAccessor* accessor_, QWidget* parent)
    : QWidget(parent)
    , accessor(accessor_)
{
    InitUI();
    BindFields();

    newFolderAction = new QAction(tr("Create folder"), this);
    connect(newFolderAction, &QAction::triggered, this, &FileSystemWidget::onNewFolder);

    newFileAction = new QAction(tr("Create file"), this);
    connect(newFileAction, &QAction::triggered, this, &FileSystemWidget::onNewFile);
    deleteAction = new QAction(tr("Delete"), this);
#if defined Q_OS_WIN
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
#elif defined Q_OS_MAC
    deleteAction->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
#endif // platform
    deleteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(deleteAction, &QAction::triggered, this, &FileSystemWidget::onDeleteFile);
    
#if defined Q_OS_WIN
    QString actionName = tr("Show in explorer");
#elif defined Q_OS_MAC
    QString actionName = tr("Show in finder");
#endif //Q_OS_WIN //Q_OS_MAC
    showInSystemExplorerAction = new QAction(actionName, this);
    connect(showInSystemExplorerAction, &QAction::triggered, this, &FileSystemWidget::OnShowInExplorer);

    renameAction = new QAction(tr("Rename"), this);
    connect(renameAction, &QAction::triggered, this, &FileSystemWidget::OnRename);

    openFileAction = new QAction(tr("Open File"), this);
    openFileAction->setShortcuts({ QKeySequence(Qt::Key_Return), QKeySequence(Qt::Key_Enter) });
    openFileAction->setShortcutContext(Qt::WidgetShortcut);
    connect(openFileAction, &QAction::triggered, this, &FileSystemWidget::OnOpenFile);

    copyInternalPathToFileAction = new QAction(tr("Copy Internal Path"), this);
    connect(copyInternalPathToFileAction, &QAction::triggered, this, &FileSystemWidget::OnCopyInternalPathToFile);

    treeView->addAction(newFolderAction);
    treeView->addAction(newFileAction);
    treeView->addAction(deleteAction);
    treeView->addAction(showInSystemExplorerAction);
    treeView->addAction(renameAction);
    treeView->addAction(openFileAction);
    treeView->addAction(copyInternalPathToFileAction);
    RefreshActions();
}

FileSystemWidget::~FileSystemWidget() = default;

void FileSystemWidget::InitUI()
{
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(5);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    filterLine = new QLineEdit(this);
    verticalLayout->addWidget(filterLine);

    treeView = new QTreeView(this);
    verticalLayout->addWidget(treeView);

    filterLine->setClearButtonEnabled(true);

    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView, &QWidget::customContextMenuRequested, this, &FileSystemWidget::OnCustomContextMenuRequested);
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    treeView->setDragDropMode(QAbstractItemView::DragOnly);

    connect(treeView, &QTreeView::doubleClicked, this, &FileSystemWidget::onDoubleClicked);
    connect(filterLine, &QLineEdit::textChanged, this, &FileSystemWidget::setFilterFixedString);
}

void FileSystemWidget::BindFields()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    fieldBinder.reset(new FieldBinder(accessor));

    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &FileSystemWidget::OnProjectPathChanged));
    }
}

void FileSystemWidget::SetResourceDirectory(const QString& path)
{
    bool isAvailable = !path.isEmpty();
    if (isAvailable)
    {
        QFileInfo fileInfo(path);
        isAvailable = fileInfo.isDir() && fileInfo.exists();
    }

    if (isAvailable)
    {
        model = new FileSystemModel(this);
        model->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
        setFilterFixedString("");
        model->setNameFilterDisables(false);
        model->setReadOnly(false);
        treeView->setModel(model);
        connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileSystemWidget::OnSelectionChanged);

        treeView->hideColumn(1);
        treeView->hideColumn(2);
        treeView->hideColumn(3);
        treeView->setRootIndex(model->setRootPath(path));
    }
    else
    {
        treeView->setModel(nullptr);
        delete model;
        model = nullptr;
        treeView->hideColumn(0);
    }
}

void FileSystemWidget::SelectFile(const QString& filePath)
{
    DVASSERT(!filePath.isEmpty());
    treeView->setCurrentIndex(model->index(filePath));
}

//refresh actions by menu invoke pos
void FileSystemWidget::RefreshActions()
{
    bool isProjectOpened = !treeView->isColumnHidden(0); //column is hidden if no open projects
    bool canCreateFile = isProjectOpened;
    bool canCreateDir = isProjectOpened;
    bool canShow = false;
    bool canRename = false;
    bool canCopyInternalPath = false;
    const QModelIndex& index = treeView->indexAt(menuInvokePos);

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

bool FileSystemWidget::CanDelete(const QModelIndex& index) const
{
    if (!model->isDir(index))
    {
        return true;
    }
    QDir dir(model->filePath(index));
    QDirIterator dirIterator(dir, QDirIterator::Subdirectories);
    while (dirIterator.hasNext())
    {
        if (dirIterator.next().endsWith(Project::GetUiFileExtension()))
        {
            return false;
        }
    }
    return true;
}

QString FileSystemWidget::GetPathByCurrentPos(ePathType pathType)
{
    QModelIndex index = treeView->indexAt(menuInvokePos);
    QString path;
    if (!index.isValid())
    {
        path = model->rootPath();
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

void FileSystemWidget::onDoubleClicked(const QModelIndex& index)
{
    if (!model->isDir(index))
    {
        openFile.Emit(model->filePath(index));
    }
}

void FileSystemWidget::setFilterFixedString(const QString& filterStr)
{
    QStringList filters;
    filters << QString("*%1*" + Project::GetUiFileExtension()).arg(filterStr);
    model->setNameFilters(filters);
}

void FileSystemWidget::onNewFolder()
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
        QModelIndex index = treeView->indexAt(menuInvokePos);
        if (!index.isValid())
        {
            index = treeView->rootIndex();
        }
        model->mkdir(index, folderName);
    }
    RefreshActions();
}

void FileSystemWidget::onNewFile()
{
    auto path = GetPathByCurrentPos(DirPath);
    QString strFile = FileDialog::getSaveFileName(this, tr("Create new file"), path, "*" + Project::GetUiFileExtension());
    if (strFile.isEmpty())
    {
        return;
    }
    if (!strFile.endsWith(Project::GetUiFileExtension()))
    {
        strFile += Project::GetUiFileExtension();
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

void FileSystemWidget::onDeleteFile()
{
    QModelIndex index;
    if (menuInvokePos.x() != -1 && menuInvokePos.y() != -1)
    {
        index = treeView->indexAt(menuInvokePos);
    }
    else
    {
        const auto& indexes = treeView->selectionModel()->selectedIndexes();
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

void FileSystemWidget::OnShowInExplorer()
{
    auto pathIn = GetPathByCurrentPos(AnyPath);
    QtHelpers::ShowInOSFileManager(pathIn);
}

void FileSystemWidget::OnRename()
{
    auto index = treeView->indexAt(menuInvokePos);
    treeView->edit(index);
}

void FileSystemWidget::OnOpenFile()
{
    const auto& indexes = treeView->selectionModel()->selectedIndexes();
    for (auto index : indexes)
    {
        if (!model->isDir(index))
        {
            openFile.Emit(model->filePath(index));
        }
    }
}

void FileSystemWidget::OnCopyInternalPathToFile()
{
    const QModelIndexList& indexes = treeView->selectionModel()->selectedIndexes();
    for (const QModelIndex& index : indexes)
    {
        DAVA::FilePath path = model->filePath(index).toStdString();

        QClipboard* clipboard = QApplication::clipboard();
        QMimeData* data = new QMimeData();
        data->setText(QString::fromStdString(path.GetFrameworkPath()));
        clipboard->setMimeData(data);
    }
}

void FileSystemWidget::OnCustomContextMenuRequested(const QPoint& pos)
{
    menuInvokePos = pos;
    RefreshActions();
    QMenu::exec(treeView->actions(), treeView->viewport()->mapToGlobal(pos));
    menuInvokePos = QPoint(-1, -1);
}

void FileSystemWidget::OnSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    const QModelIndexList& indexes = treeView->selectionModel()->selectedIndexes();
    UpdateActionsWithShortcutsState(indexes);
}

void FileSystemWidget::UpdateActionsWithShortcutsState(const QModelIndexList& indexes)
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

void FileSystemWidget::OnProjectPathChanged(const DAVA::Any& projectPath)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* globalContext = accessor->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();

    setEnabled(projectData != nullptr);

    if (projectData == nullptr)
    {
        SetResourceDirectory(QString());
    }
    else
    {
        FilePath uiDirectory = projectData->GetUiDirectory().absolute;
        const EngineContext* context = GetEngineContext();
        DVASSERT(context->fileSystem->IsDirectory(uiDirectory));
        QString uiResourcesPath = QString::fromStdString(uiDirectory.GetStringValue());

        SetResourceDirectory(uiResourcesPath);
    }
}
