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


#include "WidgetItem.h"

#include <QWidget>
#include <QEvent>
#include <QDebug>
#include <QMetaObject>

#include "WidgetModel.h"



WidgetItem::WidgetItem( QWidget* w )
    : QObject( nullptr )
    , widget( w )
{
    if ( !widget.isNull() )
    {
        widget->installEventFilter( this );
    }
}

WidgetItem::~WidgetItem()
{
    if ( !widget.isNull() )
    {
        widget->removeEventFilter( this );

        if ( !model.isNull() )
        {
            model->cache.remove( widget );
        }
    }
}

void WidgetItem::rebuildChildren()
{
    children.clear();
    auto childrenWidgets = widget->findChildren<QWidget *>( QString(), Qt::FindDirectChildrenOnly );

    children.reserve( childrenWidgets.size() );
    for ( auto childWidget : childrenWidgets )
    {
        auto childItem = create( childWidget );
        childItem->parentItem = self;
        childItem->rebuildChildren();
        children << childItem;
    }
}

void WidgetItem::onChildAdd( QWidget* w )
{
    if ( w == nullptr )
        return;

    auto childItem = create( w );
    childItem->parentItem = self;
    childItem->rebuildChildren();

    children << childItem;

    if ( !model.isNull() )
    {
        model->cache[w] = childItem;
        auto parentIndex = model->indexFromWidget( w->parentWidget() );
        model->beginInsertRows( parentIndex, children.size(), children.size() );
        model->endInsertRows();
    }
}

void WidgetItem::onChildRemove( QWidget* w )
{
    if ( w == nullptr )
        return;

    for ( auto it = children.begin(); it != children.end(); ++it )
    {
        auto child = it->data()->widget;
        if ( child == w )
        {
            auto index = it - children.begin();
            children.erase( it );

            if ( !model.isNull() )
            {
                model->cache.remove( w );
                auto parentIndex = model->indexFromWidget( w->parentWidget() );
                model->beginRemoveRows( parentIndex, index, index );
                model->endRemoveRows();
            }

            return;
        }
    }
}

bool WidgetItem::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == widget.data() )
    {
        switch ( e->type() )
        {
        case QEvent::ChildAdded:
            {
                auto event = static_cast<QChildEvent *>( e );
                auto w = qobject_cast< QWidget *>( event->child() );
                onChildAdd( w );
            }
            break;
        case QEvent::ChildRemoved:
            {
                auto event = static_cast<QChildEvent *>( e );
                auto w = qobject_cast< QWidget *>( event->child() );
                onChildRemove( w );
            }
            break;

        default:
            break;
        }
    }

    return QObject::eventFilter( obj, e );
}

QSharedPointer<WidgetItem> WidgetItem::create( QWidget* w )
{
    auto item = QSharedPointer< WidgetItem >::create( w );
    item->self = item.toWeakRef();

    return item;
}
