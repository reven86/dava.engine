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


#ifndef __ABSTRACTPROPERTYDELEGATE_H__
#define __ABSTRACTPROPERTYDELEGATE_H__

#include <QList>
#include <QPointer>

class PropertiesTreeItemDelegate;
class QAction;
class QWidget;
class QModelIndex;
class QStyleOptionViewItem;
class QAbstractItemModel;

class AbstractPropertyDelegate
{
public:
    explicit AbstractPropertyDelegate(PropertiesTreeItemDelegate *delegate = NULL);
    virtual ~AbstractPropertyDelegate();

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) = 0;
    virtual void enumEditorActions(QWidget *parent, const QModelIndex &index, QList<QAction *> &actions) = 0;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const = 0;
    virtual bool setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const = 0;

protected:
    QPointer<PropertiesTreeItemDelegate> itemDelegate;

};

/*
class PropertyAbstractEditor: public QObject
{
Q_OBJECT
public:
explicit PropertyAbstractEditor(PropertiesTreeItemDelegate *delegate = NULL);
virtual ~PropertyAbstractEditor();

virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const { return NULL; };
virtual void addEditorWidgets(QWidget *parent, const QModelIndex &index) const;
virtual QList<QAction *> enumEditorActions(QWidget *parent, const QModelIndex &index) const;
virtual void setEditorData(QWidget *editor, const QModelIndex &index) const = 0;
virtual bool setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
virtual void paint( QPainter * painter, QStyleOptionViewItem & option, const QModelIndex & index ) const{}

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

protected:
PropertiesTreeItemDelegate *itemDelegate;

};
*/
#endif // __ABSTRACTPROPERTYDELEGATE_H__
