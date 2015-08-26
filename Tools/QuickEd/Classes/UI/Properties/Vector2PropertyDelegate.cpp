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


#include "Vector2PropertyDelegate.h"

#include <QLineEdit>
#include <QLayout>
#include "PropertiesModel.h"
#include "Utils/QtDavaConvertion.h"
#include "Utils/Utils.h"
#include "PropertiesTreeItemDelegate.h"

using namespace DAVA;

Vector2PropertyDelegate::Vector2PropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{
}

Vector2PropertyDelegate::~Vector2PropertyDelegate()
{
}

QWidget * Vector2PropertyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, &QLineEdit::editingFinished, this, &Vector2PropertyDelegate::OnEditingFinished);
    return lineEdit;
}

void Vector2PropertyDelegate::setEditorData(QWidget * rawEditor, const QModelIndex & index) const
{
    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");
    
    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    QString stringValue;
    if (variant.GetType() == VariantType::TYPE_VECTOR2)
    {
        const Vector2 &v = variant.AsVector2();
        stringValue.QString::sprintf("%g; %g", v.x, v.y);
    }
    else
    {
        stringValue = "?";
        DVASSERT(false);
    }
    editor->blockSignals(true);
    editor->setText(stringValue);
    editor->blockSignals(false);
}

bool Vector2PropertyDelegate::setModelData(QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;
    
    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");
    
    DAVA::VariantType variantType;
    
    QStringList tokens = editor->text().split(";");
    
    Vector2 val;
    int count = Min(tokens.size(), 2);
    for (int i = 0; i < count; i++)
    {
        val.data[i] = tokens[i].toFloat();
    }
    
    QVariant variant;
    variant.setValue<DAVA::VariantType>(VariantType(val));
    
    return model->setData(index, variant, Qt::EditRole);
}

void Vector2PropertyDelegate::OnEditingFinished()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender());
    if (!lineEdit)
        return;
    
    QWidget *editor = lineEdit->parentWidget();
    if (!editor)
        return;
    
    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
}
