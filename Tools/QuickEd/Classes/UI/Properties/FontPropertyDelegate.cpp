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


#include "FontPropertyDelegate.h"
#include <DAVAEngine.h>
#include <QAction>
#include <QComboBox>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "EditorCore.h"
#include "UI/Dialogs/DialogConfigurePreset.h"
#include "UI/Dialogs/DialogAddPreset.h"

using namespace DAVA;


FontPropertyDelegate::FontPropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : BasePropertyDelegate(delegate)
{

}

FontPropertyDelegate::~FontPropertyDelegate()
{

}

QWidget * FontPropertyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->setObjectName("comboBox");
    comboBox->addItem("");
    comboBox->addItems(GetEditorFontSystem()->GetDefaultPresetNames());
    connect(comboBox, &QComboBox::currentTextChanged, this, &FontPropertyDelegate::valueChanged);
    return comboBox;
}

void FontPropertyDelegate::setEditorData(QWidget *rawEditor, const QModelIndex &index) const
{
    QComboBox *editor = rawEditor->findChild<QComboBox*>("comboBox");

    VariantType variant = index.data(Qt::EditRole).value<VariantType>();
    editor->setCurrentText(QString::fromStdString(variant.AsString()));
}

bool FontPropertyDelegate::setModelData(QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QComboBox *editor = rawEditor->findChild<QComboBox*>("comboBox");

    VariantType variantType = index.data(Qt::EditRole).value<VariantType>();
    String str = QStringToString(editor->currentText());
    variantType.SetString(str);

    QVariant variant;
    variant.setValue<VariantType>(variantType);

    return model->setData(index, variant, Qt::EditRole);
}

void FontPropertyDelegate::enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions)
{
    configurePresetAction = new QAction(QIcon(":/Icons/configure.png"), tr("configure"), parent);
    configurePresetAction->setEnabled(false);
    configurePresetAction->setToolTip(tr("configure preset"));
    actions.push_back(configurePresetAction);
    connect(configurePresetAction, &QAction::triggered, this, &FontPropertyDelegate::configurePresetClicked);

    QAction *addPresetAction = new QAction(QIcon(":/Icons/add.png"), tr("configure"), parent);
    addPresetAction->setToolTip(tr("add preset"));
    actions.push_back(addPresetAction);
    connect(addPresetAction, &QAction::triggered, this, &FontPropertyDelegate::addPresetClicked);

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void FontPropertyDelegate::addPresetClicked()
{
    QAction *editPresetAction = qobject_cast<QAction *>(sender());
    if (!editPresetAction)
        return;

    QWidget *editor = editPresetAction->parentWidget();
    if (!editor)
        return;
    QComboBox *comboBox = editor->findChild<QComboBox*>("comboBox");
    DialogAddPreset dialogAddPreset(comboBox->currentText(), qApp->activeWindow());
    if (dialogAddPreset.exec())
    {
        comboBox->clear();
        comboBox->addItems(GetEditorFontSystem()->GetDefaultPresetNames());
        comboBox->setCurrentText(dialogAddPreset.lineEdit_newFontPresetName->text());

        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void FontPropertyDelegate::configurePresetClicked()
{
    QAction *editPresetAction = qobject_cast<QAction *>(sender());
    if (!editPresetAction)
        return;

    QWidget *editor = editPresetAction->parentWidget();
    if (!editor)
        return;

    QComboBox *comboBox = editor->findChild<QComboBox*>("comboBox");
    DialogConfigurePreset dialogConfigurePreset(comboBox->currentText(), qApp->activeWindow());
    if (dialogConfigurePreset.exec())
    {
        BasePropertyDelegate::SetValueModified(editor, true);
        itemDelegate->emitCommitData(editor);
    }
}

void FontPropertyDelegate::valueChanged()
{
    QComboBox *comboBox= qobject_cast<QComboBox *>(sender());
    configurePresetAction->setDisabled(comboBox->currentText().isEmpty());
    if (nullptr == comboBox)
    {
        return;
    }
    QWidget *editor = comboBox->parentWidget();
    if (nullptr == editor)
    {
        return;
    }

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}