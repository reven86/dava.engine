#include "UI/FileSystemView/FindFileInPackageDialog.h"
#include "ui_FindFileInPackageDialog.h"

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

QString FindFileInPackageDialog::GetFilePath(const ProjectStructure* projectStructure, const DAVA::String& suffix, QWidget* parent)
{
    DAVA::Vector<DAVA::FilePath> files = projectStructure->GetFiles(suffix);
    DVASSERT(!files.empty());

    FindFileInPackageDialog dialog(files, parent);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString filePath = dialog.ui->lineEdit->text();
        QString absFilePath = dialog.prefix + filePath;
        QFileInfo fileInfo(absFilePath);
        DVASSERT(fileInfo.isFile() && fileInfo.suffix() == QString::fromStdString(suffix));
        return absFilePath;
    }
    return QString();
}

FindFileInPackageDialog::FindFileInPackageDialog(const DAVA::Vector<DAVA::FilePath>& files, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::FindFileInPackageDialog())
{
    ui->setupUi(this);
    ui->lineEdit->setFocus();

    installEventFilter(this);
    ui->lineEdit->installEventFilter(this);

    Init(files);
}

void FindFileInPackageDialog::Init(const DAVA::Vector<DAVA::FilePath>& files)
{
    //calculate common prefix for all given files
    //initialize prefix with first element
    prefix = QString::fromStdString(files.front().GetAbsolutePathname());
    for (const DAVA::FilePath& filePath : files)
    {
        QString absPath = QString::fromStdString(filePath.GetAbsolutePathname());
        int index = 0;
        for (int i = 0, count = qMin(absPath.size(), prefix.size()); i < count && prefix[i] == absPath[i]; ++i, ++index)
        {
        }
        prefix.truncate(index);
    }

    //collect all items in short form
    QStringList stringsToDisplay;
    const int prefixSize = prefix.size();
    for (const DAVA::FilePath& filePath : files)
    {
        QString absPath = QString::fromStdString(filePath.GetAbsolutePathname());
        const int relPathSize(absPath.size() - prefixSize);
        stringsToDisplay << absPath.right(relPathSize);
    }

    //the only way to not create model and use stringlist is a pass stringlist to the QCompleter c-tor :(
    completer = new QCompleter(stringsToDisplay, this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->installEventFilter(this);
    completer->popup()->installEventFilter(this);
    completer->popup()->setTextElideMode(Qt::ElideRight);

    ui->lineEdit->setCompleter(completer);
}

bool FindFileInPackageDialog::eventFilter(QObject* obj, QEvent* event)
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
