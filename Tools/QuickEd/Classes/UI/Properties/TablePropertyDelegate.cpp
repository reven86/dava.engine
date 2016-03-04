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


#include "TablePropertyDelegate.h"

#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "EditorCore.h"
#include "UI/Dialogs/TableEditorDialog.h"

using namespace DAVA;

TablePropertyDelegate::TablePropertyDelegate(PropertiesTreeItemDelegate* delegate)
    : BasePropertyDelegate(delegate)
{
}

TablePropertyDelegate::~TablePropertyDelegate()
{
}

QWidget* TablePropertyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QLineEdit* lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName("lineEdit");
    connect(lineEdit, &QLineEdit::textChanged, this, &TablePropertyDelegate::valueChanged);
    return lineEdit;
}

void TablePropertyDelegate::setEditorData(QWidget* rawEditor, const QModelIndex& index) const
{
    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    VariantType variant = index.data(Qt::EditRole).value<VariantType>();
    editor->setText(QString::fromStdString(variant.AsString()));
}

bool TablePropertyDelegate::setModelData(QWidget* rawEditor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    VariantType variantType = index.data(Qt::EditRole).value<VariantType>();
    String str = QStringToString(editor->text());
    variantType.SetString(str);

    QVariant variant;
    variant.setValue<VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void TablePropertyDelegate::enumEditorActions(QWidget* parent, const QModelIndex& index, QList<QAction*>& actions)
{
    editTableAction = new QAction(QIcon(":/Icons/configure.png"), tr("edit"), parent);
    editTableAction->setToolTip(tr("edit table"));
    actions.push_back(editTableAction);
    connect(editTableAction, &QAction::triggered, this, &TablePropertyDelegate::editTableClicked);

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void TablePropertyDelegate::editTableClicked()
{
    QAction* editPresetAction = qobject_cast<QAction*>(sender());
    if (!editPresetAction)
        return;

    QWidget* rawEditor = editPresetAction->parentWidget();
    if (!rawEditor)
        return;

    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");
    TableEditorDialog dialog(editor->text(), qApp->activeWindow());
    if (dialog.exec())
    {
        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void TablePropertyDelegate::valueChanged(const QString& newStr)
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if (lineEdit == nullptr)
    {
        return;
    }

    QWidget* editor = lineEdit->parentWidget();
    if (nullptr == editor)
    {
        return;
    }

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
