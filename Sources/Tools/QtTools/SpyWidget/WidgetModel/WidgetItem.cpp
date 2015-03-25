#include "WidgetItem.h"

#include <QWidget>
#include <QEvent>
#include <QAbstractItemModel>
#include <QDebug>
#include <QMetaObject>


WidgetItem::WidgetItem( QWidget* w )
    : QObject( w )
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
            return;
        }
    }

    Q_ASSERT( false );
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
                // onChildAdd( w );
            }
            break;
        case QEvent::ChildRemoved:
            {
                auto event = static_cast<QChildEvent *>( e );
                auto w = qobject_cast< QWidget *>( event->child() );
                // onChildRemove( w );
            }
            break;

        default:
            break;
        }
    }

    return QObject::eventFilter( obj, e );
}

QSharedPointer<WidgetItem> WidgetItem::getParent() const
{
    return parentItem;
}

QSharedPointer<WidgetItem> WidgetItem::create( QWidget* w )
{
    auto item = QSharedPointer< WidgetItem >::create( w );
    item->self = item.toWeakRef();

    return item;
}
