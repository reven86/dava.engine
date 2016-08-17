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
    if (dialog.exec() == QDialog::Accepted)
    {
        return dialog.filePath;
    }
    return "";
}

void FindFileInPackageDialog::InitFromPath(const QString& path)
{
    QStringList availableFiles;
    QFileInfo fileInfo(path);
    qApp->setOverrideCursor(Qt::BusyCursor);
    if (fileInfo.exists() && fileInfo.isDir())
    {
        QDirIterator iter(path, QDirIterator::Subdirectories);
        while (iter.hasNext())
        {
            QFileInfo info(iter.fileInfo());
            if (info.isFile() && info.suffix() == "yaml")
            {
                availableFiles << info.absoluteFilePath();
            }
            iter.next();
        }
    }

    qApp->restoreOverrideCursor();
    QCompleter* completer = new QCompleter(availableFiles, this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    QLineEdit* lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Start typing a file name...");
    lineEdit->setCompleter(completer);
    connect(completer, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted), lineEdit, &QLineEdit::setPlaceholderText);

    //make one-way binding from lineEdit to the class member;
    connect(lineEdit, &QLineEdit::textChanged, [this](QString text) { filePath = text; });
    layout()->addWidget(lineEdit);
}
