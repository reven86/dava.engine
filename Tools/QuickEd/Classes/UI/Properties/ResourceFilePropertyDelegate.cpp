/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "ResourceFilePropertyDelegate.h"
#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "QtTools/FileDialog/FileDialog.h"
#include "ResourcesManageHelper.h"

using namespace DAVA;


ResourceFilePropertyDelegate::ResourceFilePropertyDelegate(const QString &aResourcefilter, const QString &aResourceDir, PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
    , resourcefilter(aResourcefilter)
    , resourceDir(aResourceDir)
{

}

ResourceFilePropertyDelegate::~ResourceFilePropertyDelegate()
{

}

QWidget * ResourceFilePropertyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(valueChanged()));

    return lineEdit;
}

void ResourceFilePropertyDelegate::setEditorData(QWidget * rawEditor, const QModelIndex & index) const
{
    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    QString stringValue = StringToQString(variant.AsFilePath().GetStringValue());
    editor->setText(stringValue);
}

bool ResourceFilePropertyDelegate::setModelData(QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();
    DAVA::FilePath absoluteFilePath = QStringToString(editor->text());
    DAVA::FilePath frameworkFilePath = absoluteFilePath.GetFrameworkPath();
    variantType.SetFilePath(frameworkFilePath);

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void ResourceFilePropertyDelegate::enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions)
{
    QAction *selectFileAction = new QAction(tr("..."), parent);
    selectFileAction->setToolTip(tr("Select resource file"));
    actions.push_back(selectFileAction);
    connect(selectFileAction, SIGNAL(triggered(bool)), this, SLOT(selectFileClicked()));

    QAction *clearFileAction = new QAction(QIcon(":/Icons/editclear.png"), tr("clear"), parent);
    clearFileAction->setToolTip(tr("Clear resource file"));
    actions.push_back(clearFileAction);
    connect(clearFileAction, SIGNAL(triggered(bool)), this, SLOT(clearFileClicked()));

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void ResourceFilePropertyDelegate::selectFileClicked()
{
    QAction *openFileDialogAction = qobject_cast<QAction *>(sender());
    if (!openFileDialogAction)
        return;

    QWidget *editor = openFileDialogAction->parentWidget();
    if (!editor)
        return;

    QLineEdit *lineEdit = editor->findChild<QLineEdit*>("lineEdit");

    QString dir;
    QString pathText = lineEdit->text();
    if (!pathText.isEmpty())
    {
        DAVA::FilePath filePath = QStringToString(pathText);
        dir = StringToQString(filePath.GetDirectory().GetAbsolutePathname());
    }
    else
    {
        dir = ResourcesManageHelper::GetResourceRootDirectory() + resourceDir;
    }

    QString filePathText = FileDialog::getOpenFileName(editor->parentWidget(), tr("Select resource file"), dir, resourcefilter);
    if (!filePathText.isEmpty())
    {
        DAVA::FilePath absoluteFilePath = QStringToString(filePathText);
        DAVA::FilePath frameworkFilePath = absoluteFilePath.GetFrameworkPath();
        lineEdit->setText(StringToQString(frameworkFilePath.GetStringValue()));

        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void ResourceFilePropertyDelegate::clearFileClicked()
{
    QAction *clearSpriteAction = qobject_cast<QAction *>(sender());
    if (!clearSpriteAction)
        return;

    QWidget *editor = clearSpriteAction->parentWidget();
    if (!editor)
        return;

    QLineEdit *lineEdit = editor->findChild<QLineEdit*>("lineEdit");

    lineEdit->setText(QString(""));

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}

void ResourceFilePropertyDelegate::valueChanged()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (!lineEdit)
        return;

    QWidget *editor = lineEdit->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
}
