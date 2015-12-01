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


#include "IntegerPropertyDelegate.h"
#include <QSpinBox>
#include <QLayout>
#include "FileSystem/VariantType.h"
#include "PropertiesModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"

IntegerPropertyDelegate::IntegerPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{

}

IntegerPropertyDelegate::~IntegerPropertyDelegate()
{

}

QWidget * IntegerPropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index )
{
    QSpinBox *spinBox = new QSpinBox(parent);
    spinBox->setObjectName(QString::fromUtf8("spinBox"));
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));

    return spinBox;
}

void IntegerPropertyDelegate::setEditorData( QWidget * rawEditor, const QModelIndex & index ) const 
{
    QSpinBox *editor = rawEditor->findChild<QSpinBox*>("spinBox");

    editor->blockSignals(true);
    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    editor->setMinimum(-99999);
    editor->setMaximum(99999);
    switch (variant.GetType())
    {
    case DAVA::VariantType::TYPE_INT32:
        editor->setValue(variant.AsInt32());
        break;
    case DAVA::VariantType::TYPE_INT64:
        editor->setValue(variant.AsInt64());
        break;
    case DAVA::VariantType::TYPE_UINT32:
        editor->setMinimum(0);
        editor->setValue(variant.AsUInt32());
        break;
    case DAVA::VariantType::TYPE_UINT64:
        editor->setMinimum(0);
        editor->setValue(variant.AsUInt64());
        break;
    default:
        break;
    }
    editor->blockSignals(false);
    BasePropertyDelegate::SetValueModified(editor, false);
}

bool IntegerPropertyDelegate::setModelData( QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QSpinBox *editor = rawEditor->findChild<QSpinBox*>("spinBox");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();

    switch (variantType.GetType())
    {
    case DAVA::VariantType::TYPE_INT32:
        variantType.SetInt32(editor->value());
        break;
    case DAVA::VariantType::TYPE_INT64:
        variantType.SetInt64(editor->value());
        break;
    case DAVA::VariantType::TYPE_UINT32:
        variantType.SetUInt32(editor->value());
        break;
    case DAVA::VariantType::TYPE_UINT64:
        variantType.SetUInt64(editor->value());
        break;
    default:
        break;
    }

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void IntegerPropertyDelegate::OnValueChanged()
{
    QWidget *spinBox = qobject_cast<QWidget *>(sender());
    if (!spinBox)
        return;

    QWidget *editor = spinBox->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}

