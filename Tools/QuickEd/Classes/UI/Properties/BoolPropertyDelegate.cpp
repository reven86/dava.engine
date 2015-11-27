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


#include "BoolPropertyDelegate.h"
#include <QComboBox>
#include "DAVAEngine.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"


BoolPropertyDelegate::BoolPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{

}

BoolPropertyDelegate::~BoolPropertyDelegate()
{

}

QWidget * BoolPropertyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->setObjectName(QString::fromUtf8("comboBox"));
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged()));

    comboBox->blockSignals(true);
    comboBox->addItem(QVariant(false).toString(), QVariant(false));
    comboBox->addItem(QVariant(true).toString(), QVariant(true));
    comboBox->blockSignals(false);

    return comboBox;
}

void BoolPropertyDelegate::setEditorData(QWidget * rawEditor, const QModelIndex & index) const
{
    QComboBox *editor = rawEditor->findChild<QComboBox *>("comboBox");

    editor->blockSignals(true);
    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    DVASSERT(variant.GetType() == DAVA::VariantType::TYPE_BOOLEAN);
    int comboIndex = editor->findData(QVariant(variant.AsBool()));
    editor->setCurrentIndex(comboIndex);
    editor->blockSignals(false);

    BasePropertyDelegate::SetValueModified(editor, false);
}

bool BoolPropertyDelegate::setModelData(QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QComboBox *comboBox = rawEditor->findChild<QComboBox *>("comboBox");

    DAVA::VariantType variantType(comboBox->itemData(comboBox->currentIndex()).toBool());
    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void BoolPropertyDelegate::OnCurrentIndexChanged()
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
