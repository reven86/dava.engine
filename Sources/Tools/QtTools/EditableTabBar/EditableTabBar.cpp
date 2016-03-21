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

#include "EditableTabBar.h"
#include "Debug/DVAssert.h"

#include <QLineEdit>
#include <QEvent>
#include <QMouseEvent>
#include <QAction>

EditableTabBar::EditableTabBar(QWidget* parent)
    : QTabBar(parent)
    , nameEditor(new QLineEdit(this))
{
    nameEditor->setWindowFlags(Qt::Popup);
    nameEditor->installEventFilter(this);
    DVVERIFY(QObject::connect(nameEditor, &QLineEdit::editingFinished, this, &EditableTabBar::onNameEditingFinished));

    DVVERIFY(QObject::connect(this, &QTabBar::tabBarDoubleClicked, this, &EditableTabBar::onTabDoubleClicked));
}

void EditableTabBar::setNameValidator(const QValidator* v)
{
    DVASSERT(nameEditor != nullptr);
    nameEditor->setValidator(v);
}

bool EditableTabBar::isEditable() const
{
    return isTabsEditable;
}

void EditableTabBar::setEditable(bool isEditable_)
{
    isTabsEditable = isEditable_;
}

bool EditableTabBar::eventFilter(QObject* object, QEvent* event)
{
    if (nameEditor->isVisible())
    {
        DVASSERT(isEditable());
        bool hideEditor = false;
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
            hideEditor = !nameEditor->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos());
            break;
        case QEvent::KeyPress:
            hideEditor = static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape;
            break;
        default:
            break;
        }

        if (hideEditor)
        {
            finishEdit(false);
            return true;
        }
    }
    return QTabBar::eventFilter(object, event);
}

void EditableTabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);
}

void EditableTabBar::onNameEditingFinished()
{
    finishEdit(true);
}

void EditableTabBar::onTabDoubleClicked(int index)
{
    startEdit(index);
}

void EditableTabBar::startEdit(int tabIndex)
{
    if (isTabsEditable == false)
    {
        return;
    }

    QRect rect = tabRect(tabIndex);
    nameEditor->setFixedSize(rect.size());
    nameEditor->move(mapToGlobal(rect.topLeft()));
    QString text = tabText(tabIndex);
    nameEditor->setText(text);
    nameEditor->setSelection(0, text.size());
    if (!nameEditor->isVisible())
    {
        nameEditor->show();
    }
}

void EditableTabBar::finishEdit(bool commitChanges)
{
    DVASSERT(isEditable());

    nameEditor->hide();
    if (commitChanges)
    {
        QString newName = nameEditor->text();
        int editedTab = currentIndex();
        if (editedTab >= 0)
        {
            setTabText(editedTab, newName);
            emit tabNameChanged(editedTab);
        }
    }
}
