#include "QtTools/FileDialogs/FindFileDialog.h"
#include "ui_FindFileDialog.h"

#include "Debug/DVAssert.h"
#include "FileSystem/FilePath.h"

#include "QtTools/ProjectInformation/ProjectStructure.h"

#include <QHBoxLayout>
#include <QCompleter>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QAbstractItemView>
#include <QKeyEvent>

QString FindFileDialog::GetFilePath(const ProjectStructure* projectStructure, const DAVA::String& suffix, QWidget* parent)
{
    //Qt::Popup do not prevent us to show another dialog
    static bool shown;
    if (shown)
    {
        return QString();
    }
    shown = true;

    FindFileDialog dialog(projectStructure, suffix, parent);
    dialog.setModal(true);
    int retCode = dialog.exec();

    shown = false;
    if (retCode == QDialog::Accepted)
    {
        QString filePath = dialog.ui->lineEdit->text();
        filePath = dialog.FromShortName(filePath);
        QFileInfo fileInfo(filePath);
        if (fileInfo.isFile() && fileInfo.suffix().toLower() == QString::fromStdString(suffix).toLower())
        {
            return filePath;
        }
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

FindFileDialog::FindFileDialog(const ProjectStructure* projectStructure, const DAVA::String& suffix, QWidget* parent)
    : QDialog(parent, Qt::Popup)
    , ui(new Ui::FindFileDialog())
{
    DAVA::Vector<DAVA::FilePath> files = projectStructure->GetFiles(suffix);
    DVASSERT(!files.empty());

    DAVA::String prefixStr = projectStructure->GetProjectDirectory().GetAbsolutePathname();
    prefix = QString::fromStdString(prefixStr);

    ui->setupUi(this);
    ui->lineEdit->setFocus();

    installEventFilter(this);
    ui->lineEdit->installEventFilter(this);

    Init(files);
}

void FindFileDialog::Init(const DAVA::Vector<DAVA::FilePath>& files)
{
    //collect all items in short form
    QStringList stringsToDisplay;
    for (const DAVA::FilePath& filePath : files)
    {
        QString path = QString::fromStdString(filePath.GetAbsolutePathname());
        stringsToDisplay << ToShortName(path);
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
            accept();
        }
    }
    return QDialog::eventFilter(obj, event);
}

namespace FindFileDialogDetails
{
QString newPrefix = "...";
}

QString FindFileDialog::ToShortName(const QString& name) const
{
    if (!prefix.isEmpty())
    {
        const int prefixSize = prefix.size();
        const int relPathSize = name.size() - prefixSize;
        return FindFileDialogDetails::newPrefix + name.right(relPathSize);
    }
    else
    {
        return name;
    }
}

QString FindFileDialog::FromShortName(const QString& name) const
{
    if (!prefix.isEmpty())
    {
        return prefix + name.right(name.size() - FindFileDialogDetails::newPrefix.size());
    }
    return name;
}
