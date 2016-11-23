#include "QtTools/FileDialogs/FindFileDialog.h"
#include "ui_FindFileDialog.h"

#include "Preferences/PreferencesRegistrator.h"

#include "Debug/DVAssert.h"
#include "FileSystem/FilePath.h"

#include "QtTools/ProjectInformation/FileSystemCache.h"

#include <QHBoxLayout>
#include <QCompleter>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QAbstractItemView>
#include <QKeyEvent>

using namespace DAVA;

REGISTER_PREFERENCES_ON_START(FindFileDialog,
                              PREF_ARG("lastUsedPath", String())
                              )

QString FindFileDialog::GetFilePath(const FileSystemCache* fileSystemCache, const QString& extension, QWidget* parent)
{
    //Qt::Popup do not prevent us to show another dialog
    static bool shown = false;
    if (shown)
    {
        return QString();
    }
    shown = true;

    FindFileDialog dialog(fileSystemCache, extension, parent);
    dialog.setModal(true);
    int retCode = dialog.exec();

    shown = false;
    if (retCode == QDialog::Accepted)
    {
        QString filePath = dialog.ui->lineEdit->text();
        filePath = dialog.FromShortName(filePath);
        QFileInfo fileInfo(filePath);
        if (fileInfo.isFile() && fileInfo.suffix().toLower() == extension.toLower())
        {
            dialog.lastUsedPath = filePath.toStdString();
            return filePath;
        }
        dialog.lastUsedPath = String();
    }
    return QString();
}

QAction* FindFileDialog::CreateFindInFilesAction(QWidget* parent)
{
    QAction* findInFilesAction = new QAction(tr("Find file in project"), parent);
    findInFilesAction->setShortcutContext(Qt::ApplicationShortcut);

    QList<QKeySequence> keySequences;
    keySequences << Qt::CTRL + Qt::SHIFT + Qt::Key_O;
    keySequences << Qt::ALT + Qt::SHIFT + Qt::Key_O;

    findInFilesAction->setShortcuts(keySequences);
    return findInFilesAction;
}

FindFileDialog::FindFileDialog(const FileSystemCache* projectStructure, const QString& extension, QWidget* parent)
    : QDialog(parent, Qt::Popup)
    , ui(new Ui::FindFileDialog())
{
    PreferencesStorage::Instance()->RegisterPreferences(this);
    QStringList files = projectStructure->GetFiles(extension);

    QStringList projectDirectories = projectStructure->GetTrackedDirectories();

    QString commonParent = projectDirectories.first();
    for (auto it = ++projectDirectories.begin(); it != projectDirectories.end(); ++it)
    {
        commonParent = GetCommonParent(commonParent, (*it));
    }
    DVASSERT(!commonParent.isEmpty());
    prefix = commonParent;

    ui->setupUi(this);
    ui->lineEdit->setFocus();

    installEventFilter(this);
    ui->lineEdit->installEventFilter(this);

    Init(files);

    if (files.empty())
    {
        ui->lineEdit->setPlaceholderText(tr("Project not contains files with extension %1").arg(extension));
    }
}

FindFileDialog::~FindFileDialog()
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void FindFileDialog::Init(const QStringList& files)
{
    //init function can be called only once
    DVASSERT(stringsToDisplay.isEmpty());
    //collect all items in short form
    for (const QString& filePath : files)
    {
        stringsToDisplay << ToShortName(filePath);
    }
    stringsToDisplay.sort(Qt::CaseInsensitive);
    //the only way to not create model and use stringlist is a pass stringlist to the QCompleter c-tor :(

    completer = new QCompleter(stringsToDisplay, this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->installEventFilter(this);
    completer->popup()->installEventFilter(this);
    completer->popup()->setTextElideMode(Qt::ElideLeft);

    ui->lineEdit->setCompleter(completer);

    QString lastPath = QString::fromStdString(lastUsedPath);
    lastPath = ToShortName(lastPath);
    if (!stringsToDisplay.isEmpty() && stringsToDisplay.contains(lastPath))
    {
        ui->lineEdit->setText(lastPath);
        ui->lineEdit->selectAll();
    }
}

bool FindFileDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab)
        {
            completer->complete();
            QAbstractItemView* popup = completer->popup();
            int currentRow = popup->currentIndex().row();
            if (currentRow < popup->model()->rowCount())
            {
                popup->setCurrentIndex(popup->model()->index(currentRow + 1, 0));
            }
            else
            {
                popup->setCurrentIndex(popup->model()->index(0, 0));
            }
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            //check that we need to accept first valid item
            QString currentText = ui->lineEdit->text();
            if (!currentText.isEmpty() && !stringsToDisplay.contains(currentText))
            {
                QAbstractItemModel* completionModel = completer->completionModel();

                if (completionModel->rowCount() > 0)
                {
                    //place first valid item to the lineEdit, because it's single current text holder
                    QString text = completionModel->data(completionModel->index(0, 0), completer->completionRole()).toString();
                    if (!text.isEmpty())
                    {
                        ui->lineEdit->setText(text);
                        accept();
                    }
                }
                return true;
            }
            accept();
        }
    }
    return QDialog::eventFilter(obj, event);
}

QString FindFileDialog::GetCommonParent(const QString& path1, const QString& path2)
{
    QString absPath1 = QFileInfo(path1).absoluteFilePath();
    QString commonPath = QFileInfo(path2).absoluteFilePath();

    while (!absPath1.startsWith(commonPath, Qt::CaseInsensitive))
    {
        QFileInfo fileInfo(commonPath);
        QString parentDir = fileInfo.absolutePath();
        if (commonPath == parentDir)
        {
            commonPath.clear();
            break;
        }

        commonPath = parentDir;
    }

    return commonPath;
}

namespace FindFileDialogDetails
{
QString newPrefix = QStringLiteral("...");
}

QString FindFileDialog::ToShortName(const QString& name) const
{
    if (!prefix.isEmpty())
    {
        const int prefixSize = prefix.size();
        const int relPathSize = name.size() - prefixSize;
        return FindFileDialogDetails::newPrefix + name.right(relPathSize);
    }
    return name;
}

QString FindFileDialog::FromShortName(const QString& name) const
{
    if (!prefix.isEmpty())
    {
        return prefix + name.right(name.size() - FindFileDialogDetails::newPrefix.size());
    }
    return name;
}
