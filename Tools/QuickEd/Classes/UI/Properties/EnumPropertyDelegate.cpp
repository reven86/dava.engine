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


#include "EnumPropertyDelegate.h"
#include <QComboBox>
#include <QLayout>
#include "Model/ControlProperties/AbstractProperty.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "PropertiesModel.h"

EnumPropertyDelegate::EnumPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{
}

EnumPropertyDelegate::~EnumPropertyDelegate()
{
}

QWidget * EnumPropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->setObjectName(QString::fromUtf8("comboBox"));
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged()));

    AbstractProperty *property = static_cast<AbstractProperty *>(index.internalPointer());
    const EnumMap *enumMap = property->GetEnumMap();
    DVASSERT(enumMap);

    comboBox->blockSignals(true);
    for (size_t i = 0; i < enumMap->GetCount(); ++i)
    {
        int value = 0;
        if (enumMap->GetValue(i, value))
        {
            QVariant variantValue;
            variantValue.setValue<DAVA::VariantType>(DAVA::VariantType(value));
            comboBox->addItem(QString(enumMap->ToString(value)),variantValue);
        }
    }
    comboBox->blockSignals(false);

    return comboBox;
}

void EnumPropertyDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    QComboBox *comboBox = editor->findChild<QComboBox *>("comboBox");

    comboBox->blockSignals(true);
    int comboIndex = comboBox->findText(index.data(Qt::DisplayRole).toString());
    comboBox->setCurrentIndex(comboIndex);
    comboBox->blockSignals(false);

    BasePropertyDelegate::SetValueModified(editor, false);
}

bool EnumPropertyDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(editor, model, index))
        return true;

    QComboBox *comboBox = editor->findChild<QComboBox *>("comboBox");
    return model->setData(index, comboBox->itemData(comboBox->currentIndex()), Qt::EditRole);
}

void EnumPropertyDelegate::OnCurrentIndexChanged()
{
    QWidget *comboBox = qobject_cast<QWidget *>(sender());
    if (!comboBox)
        return;

    QWidget *editor = comboBox->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}

