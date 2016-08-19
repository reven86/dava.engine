#include "UI/FileSystemView/FindFileInPackageDialog.h"
#include <QHBoxLayout>
#include <QCompleter>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QDirIterator>

FindFileInPackageDialog::FindFileInPackageDialog(const QString& rootPath, QWidget* parent)
    : QDialog(parent)
{
    installEventFilter(this);
    setLayout(new QHBoxLayout(this));
    InitFromPath(rootPath);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);


    layout()->addWidget(buttonBox);
}

QString FindFileInPackageDialog::GetFilePath(const QString& rootPath, QWidget* parent)
{
    FindFileInPackageDialog dialog(rootPath, parent);
    if (dialog.exec() == QDialog::Accepted && dialog.availableFiles.contains(dialog.filePath))
    {
        return dialog.availableFiles[dialog.filePath];
    }
    return QString();
}

void FindFileInPackageDialog::InitFromPath(const QString& path)
{
    QFileInfo fileInfo(path);
    qApp->setOverrideCursor(Qt::BusyCursor);
    QPair<int, QString> longestPath(-1, QString());
    if (fileInfo.exists() && fileInfo.isDir())
    {
        QDirIterator iter(path, QDirIterator::Subdirectories);
        while (iter.hasNext())
        {
            QFileInfo info(iter.fileInfo());
            if (info.isFile() && info.suffix() == "yaml")
            {
                QString absPath = info.absoluteFilePath();
                QString relPath = absPath.right(absPath.size() - path.size() - 1); //chop "/" symbol at begin of relPath
                int relPathSize = relPath.size();
                const int maxPathLen = 200;
                if (relPathSize > maxPathLen)
                {
                    relPath.remove(0, relPathSize - maxPathLen);
                    relPath.prepend("...");
                }
                relPathSize = relPath.size();
                if (relPathSize > longestPath.first)
                {
                    longestPath = { relPathSize, relPath };
                }
                availableFiles.insert(relPath, absPath);
            }
            iter.next();
        }
    }

    qApp->restoreOverrideCursor();
    //the only way to not create model and use stringlist is a pass stringlist to the QCompleter c-tor :(
    completer = new QCompleter(availableFiles.keys(), this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->installEventFilter(this);
    completer->popup()->installEventFilter(this);

    QLineEdit* lineEdit = new QLineEdit(this);
    lineEdit->installEventFilter(this);
    QFontMetrics fm(lineEdit->font());
    int width = fm.width(longestPath.second) + 50; //50px for scrollbar
    lineEdit->setMinimumWidth(width);
    lineEdit->setPlaceholderText("Start typing a file name...");
    lineEdit->setCompleter(completer);

    //make one-way binding from lineEdit to the class member;
    connect(lineEdit, &QLineEdit::textChanged, [this](QString text) {
        filePath = text;
    });
    layout()->addWidget(lineEdit);
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
        else if (keyEvent->key() == Qt::Key_Tab)
        {
            accept();
        }
    }
    return QDialog::eventFilter(obj, event);
}
