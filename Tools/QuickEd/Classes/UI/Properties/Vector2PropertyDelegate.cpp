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
#include <QItemEditorFactory>
#include <QLineEdit>
#include <QApplication>
#include "QtControls/Vector2DEdit.h"
#include "Model/ControlProperties/AbstractProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "PropertiesModel.h"
#include "PropertiesTreeItemDelegate.h"

Vector2PropertyDelegate::Vector2PropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{
}

Vector2PropertyDelegate::~Vector2PropertyDelegate()
{
}

QWidget * Vector2PropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    Vector2DEdit *vectorEditor = new Vector2DEdit(parent);
    vectorEditor->setObjectName(QString::fromUtf8("vectorEditor"));
    connect(vectorEditor->lineEditX(), SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
    connect(vectorEditor->lineEditY(), SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
    return vectorEditor;
}

void Vector2PropertyDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    Vector2DEdit *vectorEditor = editor->findChild<Vector2DEdit *>("vectorEditor");
    vectorEditor->setVector2D(Vector2ToQVector2D(index.data(Qt::EditRole).value<DAVA::VariantType>().AsVector2()));
}

bool Vector2PropertyDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(editor, model, index))
        return true;

    Vector2DEdit *vectorEditor = editor->findChild<Vector2DEdit *>("vectorEditor");

    DAVA::VariantType vectorType( QVector2DToVector2(vectorEditor->vector2D()) );
    QVariant vectorVariant;
    vectorVariant.setValue<DAVA::VariantType>(vectorType);
    return model->setData(index, vectorVariant, Qt::EditRole);
}

void Vector2PropertyDelegate::OnEditingFinished()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (!lineEdit)
        return;

    QWidget *editor = lineEdit->parentWidget()->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
}