#include "UI/FileSystemView/FindFileInPackageDialog.h"
#include "UI/FileSystemView/FileSystemModel.h"
#include <QHBoxLayout>
#include <QCompleter>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QDirIterator>

FindFileInPackageDialog::FindFileInPackageDialog(const QFileSystemModel* fileSystemModel, QWidget* parent)
    : QDialog(parent)
{
    DVASSERT(fileSystemModel != nullptr);
    QCompleter* completer = new QCompleter(this);
    completer->setModel(fileSystemModel);
    completer->setCompletionMode(QCompleter::PopupCompletion);

    QLineEdit* lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Start typing a file name...");
    lineEdit->setCompleter(completer);
    connect(completer, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted), lineEdit, &QLineEdit::setPlaceholderText);

    //make one-way binding from lineEdit to the class member;
    connect(lineEdit, &QLineEdit::textChanged, [this](QString text) { filePath = text; });

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setLayout(new QHBoxLayout(this));

    layout()->addWidget(lineEdit);
    layout()->addWidget(buttonBox);
}

QString FindFileInPackageDialog::GetFilePath()
{
    FindFileInPackageDialog dialog;
    if (dialog.exec() == QDialog::Accepted)
    {
        return dialog.filePath;
    }
}
