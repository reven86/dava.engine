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


#ifndef __PROPERTYABSTRACTEDITOR_H__
#define __PROPERTYABSTRACTEDITOR_H__

#include <QObject>
#include <QWidget>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include "AbstractPropertyDelegate.h"

class PropertiesTreeItemDelegate;

class BasePropertyDelegate: public QObject
                            , public AbstractPropertyDelegate
{
    Q_OBJECT
public:
    explicit BasePropertyDelegate(PropertiesTreeItemDelegate *delegate = NULL);
    virtual ~BasePropertyDelegate();
    virtual void enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) override;
    virtual bool setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    static bool IsValueModified(QWidget *editor)
    {
        QVariant data = editor->property("valueModified");
        return data.toBool();
    }

    static void SetValueModified(QWidget *editor, bool value)
    {
        editor->setProperty("valueModified", QVariant(value));
    }

    static bool IsValueReseted(QWidget *editor)
    {
        QVariant data = editor->property("valueReseted");
        return data.toBool();
    }

    static void SetValueReseted(QWidget *editor, bool value)
    {
        editor->setProperty("valueReseted", QVariant(value));
    }
private slots:
    void resetClicked();
};

#endif // __PROPERTYABSTRACTEDITOR_H__
