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


#ifndef __PROPERTIESTREEITEMDELEGATE_H__
#define __PROPERTIESTREEITEMDELEGATE_H__

#include <QWidget>
#include <QVector2D>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include "Model/ControlProperties/AbstractProperty.h"
#include "FileSystem/VariantType.h"
class AbstractPropertyDelegate;
class QToolButton;

class PropertiesTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PropertiesTreeItemDelegate(QObject *parent = NULL);
    ~PropertiesTreeItemDelegate();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    AbstractPropertyDelegate * GetCustomItemDelegateForIndex(const QModelIndex & index) const;

    void emitCommitData(QWidget * editor);
    void emitCloseEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint);

private:
    QMap<QVariant::Type, AbstractPropertyDelegate *> qvariantItemDelegates;
    QMap<AbstractProperty::ePropertyType, AbstractPropertyDelegate *> propertyItemDelegates;
    QMap<DAVA::VariantType::eVariantType, AbstractPropertyDelegate *> variantTypeItemDelegates;
    QMap<QString, AbstractPropertyDelegate *> propertyNameTypeItemDelegates;
};
class PropertyWidget: public QWidget
{
    Q_OBJECT
public:
    explicit PropertyWidget(QWidget *parent = NULL);
    ~PropertyWidget(){};

    bool event(QEvent *e) override;

    void keyPressEvent(QKeyEvent *e) override;

    void mousePressEvent(QMouseEvent *e) override;

    void mouseReleaseEvent(QMouseEvent *e) override;

    void focusInEvent(QFocusEvent *e) override;

    void focusOutEvent(QFocusEvent *e) override;

public:
    QWidget *editWidget;
};
#endif // __PROPERTIESTREEITEMDELEGATE_H__
