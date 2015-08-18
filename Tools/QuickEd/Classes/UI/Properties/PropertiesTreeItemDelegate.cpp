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


#include "PropertiesTreeItemDelegate.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QPainter>
#include <QAction>
#include <QStylePainter>
#include <QApplication>
#include <QToolButton>
#include <QEvent>
#include <QMouseEvent>
#include "QtControls/lineeditext.h"

#include "DAVAEngine.h"
#include "QtControls/Vector2DEdit.h"
#include "Model/ControlProperties/AbstractProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "Vector2PropertyDelegate.h"
#include "EnumPropertyDelegate.h"
#include "PropertiesModel.h"
#include "StringPropertyDelegate.h"
#include "FilePathPropertyDelegate.h"
#include "ColorPropertyDelegate.h"
#include "IntegerPropertyDelegate.h"
#include "FloatPropertyDelegate.h"
#include "BoolPropertyDelegate.h"
#include "ResourceFilePropertyDelegate.h"
#include "Vector4PropertyDelegate.h"

#include "FontPropertyDelegate.h"

using namespace DAVA;

PropertiesTreeItemDelegate::PropertiesTreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    propertyItemDelegates[AbstractProperty::TYPE_ENUM] = new EnumPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_VECTOR2] = new Vector2PropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_STRING] = new StringPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_COLOR] = new ColorPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_WIDE_STRING] = new StringPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT64] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FILEPATH] = new FilePathPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT32] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT64] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT32] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FLOAT] = new FloatPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_BOOLEAN] = new BoolPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_VECTOR4] = new Vector4PropertyDelegate(this);

    propertyNameTypeItemDelegates["Sprite"] = new ResourceFilePropertyDelegate("*.txt", "/Gfx/", this);
    propertyNameTypeItemDelegates["sprite"] = new ResourceFilePropertyDelegate("*.txt", "/Gfx/", this);
    propertyNameTypeItemDelegates["Effect path"] = new ResourceFilePropertyDelegate("*.sc2", "/3d/", this);
    propertyNameTypeItemDelegates["Font"] = new FontPropertyDelegate(this);
    propertyNameTypeItemDelegates["font"] = new FontPropertyDelegate(this);
}

PropertiesTreeItemDelegate::~PropertiesTreeItemDelegate()
{
    for (auto iter = propertyItemDelegates.begin(); iter != propertyItemDelegates.end(); ++iter)
    {
        SafeDelete(iter.value());
    }

    for (auto iter = variantTypeItemDelegates.begin(); iter != variantTypeItemDelegates.end(); ++iter)
    {
        SafeDelete(iter.value());
    }

    for (auto iter = propertyNameTypeItemDelegates.begin(); iter != propertyNameTypeItemDelegates.end(); ++iter)
    {
        SafeDelete(iter.value());
    }
}

void PropertiesTreeItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize PropertiesTreeItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *PropertiesTreeItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    AbstractPropertyDelegate *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        PropertyWidget *editorWidget = new PropertyWidget(parent);
        editorWidget->setObjectName(QString::fromUtf8("editorWidget"));
        QWidget *editor = currentDelegate->createEditor(editorWidget, option, index);
        if (!editor)
        {
            DAVA::SafeDelete(editorWidget);
        }
        else
        {
            editorWidget->editWidget = editor;
            editor->setFocusProxy(editorWidget);
            editorWidget->setFocusPolicy(Qt::WheelFocus);

            QHBoxLayout *horizontalLayout = new QHBoxLayout(editorWidget);
            horizontalLayout->setSpacing(1);
            horizontalLayout->setContentsMargins(0, 0, 0, 0);
            horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
            editorWidget->setLayout(horizontalLayout);

            editorWidget->setAutoFillBackground(true);
            editorWidget->setFocusProxy(editor);

            editorWidget->layout()->addWidget(editor);

            QList<QAction *> actions;
            currentDelegate->enumEditorActions(editorWidget, index, actions);

            foreach (QAction *action, actions)
            {
                QToolButton *toolButton = new QToolButton(editorWidget);
                toolButton->setDefaultAction(action);
                toolButton->setIconSize(QSize(15, 15));
                toolButton->setFocusPolicy(Qt::StrongFocus);
                editorWidget->layout()->addWidget(toolButton);
            }
        }

        return editorWidget;
    }

    if (index.data(Qt::EditRole).type() == QVariant::Bool)
        return NULL;

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void PropertiesTreeItemDelegate::setEditorData( QWidget *editor, const QModelIndex & index ) const
{
    AbstractPropertyDelegate *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        return currentDelegate->setEditorData(editor, index);
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void PropertiesTreeItemDelegate::setModelData(QWidget * editor, QAbstractItemModel *model, const QModelIndex & index) const
{
    AbstractPropertyDelegate *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        currentDelegate->setModelData(editor, model, index);
        return;
    }

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit && !lineEdit->isModified())
        return;

    QStyledItemDelegate::setModelData(editor, model, index);
}

void PropertiesTreeItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

bool PropertiesTreeItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

AbstractPropertyDelegate * PropertiesTreeItemDelegate::GetCustomItemDelegateForIndex( const QModelIndex & index ) const
{
    AbstractProperty *property = static_cast<AbstractProperty *>(index.internalPointer());
    if (property)
    {
        auto prop_iter = propertyItemDelegates.find(property->GetType());
        if (prop_iter != propertyItemDelegates.end())
            return prop_iter.value();

        auto propName_iter = propertyNameTypeItemDelegates.find(StringToQString(property->GetName()));
        if (propName_iter != propertyNameTypeItemDelegates.end())
            return propName_iter.value();
    }


    QVariant editValue = index.data(Qt::EditRole);
    if (editValue.userType() == QMetaTypeId<DAVA::VariantType>::qt_metatype_id())
    {
        DAVA::VariantType variantType = editValue.value<DAVA::VariantType>();
        QMap<DAVA::VariantType::eVariantType, AbstractPropertyDelegate *>::const_iterator var_iter = variantTypeItemDelegates.find(variantType.GetType());
        if (var_iter != variantTypeItemDelegates.end())
            return var_iter.value();
    }
    else
    {
        QMap<QVariant::Type, AbstractPropertyDelegate *>::const_iterator iter = qvariantItemDelegates.find(editValue.type());
        if (iter != qvariantItemDelegates.end())
            return iter.value();
    }

    return NULL;
}

void PropertiesTreeItemDelegate::emitCommitData(QWidget * editor)
{
    emit commitData(editor);
}

void PropertiesTreeItemDelegate::emitCloseEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
    emit closeEditor(editor, hint);
}



PropertyWidget::PropertyWidget( QWidget *parent /*= NULL*/ ) : QWidget(parent), editWidget(NULL)
{

}

bool PropertyWidget::event(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::ShortcutOverride:
         if(((QObject *)editWidget)->event(e))
             return true;
        break;

    case QEvent::InputMethod:
        return ((QObject *)editWidget)->event(e);
        break;

    default:
        break;
    }

    return QWidget::event(e);
}

void PropertyWidget::keyPressEvent( QKeyEvent *e )
{
    ((QObject *)editWidget)->event(e);
}

void PropertyWidget::mousePressEvent( QMouseEvent *e )
{
    if(e->button() != Qt::LeftButton)
        return;

    e->ignore();
}

void PropertyWidget::mouseReleaseEvent( QMouseEvent *e )
{
    e->accept();
}

void PropertyWidget::focusInEvent( QFocusEvent *e )
{
    ((QObject *)editWidget)->event(e);
    QWidget::focusInEvent(e);
}

void PropertyWidget::focusOutEvent( QFocusEvent *e )
{
    ((QObject *)editWidget)->event(e);
    QWidget::focusOutEvent(e);
}
