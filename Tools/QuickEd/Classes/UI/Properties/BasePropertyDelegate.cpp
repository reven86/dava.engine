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


#include "BasePropertyDelegate.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>

#include "PropertiesModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"

BasePropertyDelegate::BasePropertyDelegate(PropertiesTreeItemDelegate *delegate)
    : AbstractPropertyDelegate(delegate)
{

}

BasePropertyDelegate::~BasePropertyDelegate()
{
    itemDelegate = NULL;
}

bool BasePropertyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!BasePropertyDelegate::IsValueModified(editor))
        return true;

    BasePropertyDelegate::SetValueModified(editor, false);

    if (BasePropertyDelegate::IsValueReseted(editor))
    {
        BasePropertyDelegate::SetValueReseted(editor, false);
        if (model->setData(index, QVariant(), DAVA::ResetRole))
        {
            return true;
        }
    }

    return false;

}

void BasePropertyDelegate::enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions)
{
    AbstractProperty *property = static_cast<AbstractProperty *>(index.internalPointer());
    if (property && property->GetFlags() & AbstractProperty::EF_CAN_RESET )
    {
        QAction *resetAction = new QAction(QIcon(":/Icons/edit_undo.png"), tr("reset"), parent);
        resetAction->setToolTip(tr("Reset property value to default"));
        actions.push_back(resetAction);
        connect(resetAction, SIGNAL(triggered(bool)), this, SLOT(resetClicked()));
    }
}

void BasePropertyDelegate::resetClicked()
{
    QAction *resetAction = qobject_cast<QAction *>(sender());
    if (!resetAction)
        return;

    QWidget *editor = resetAction->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueReseted(editor, true);
    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
