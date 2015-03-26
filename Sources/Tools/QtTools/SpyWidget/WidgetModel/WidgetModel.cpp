#include "WidgetModel.h"

#include <QMetaObject>

#include "WidgetItem.h"


WidgetModel::WidgetModel( QWidget *w )
    : AbstractWidgetModel( w )
    , root( WidgetItem::create( w ) )
{
    root->rebuildChildren();
    rebuildCache();
}

WidgetModel::~WidgetModel()
{
}

QWidget* WidgetModel::widgetFromIndex( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return nullptr;

    auto item = static_cast<WidgetItem *>( index.internalPointer() );

    return item->widget;
}

QModelIndex WidgetModel::indexFromWidget( QWidget* w ) const
{
    if ( w == root->widget )
        return createIndex( 0, 0, root.data() );

    auto it = cache.find( w );
    if ( it == cache.end() )
        return QModelIndex();

    auto item = it.value();
    Q_ASSERT( item );
    auto p = item->parentItem;
    Q_ASSERT( !p.isNull() );    // p may be null only if it is root item. Tested at the start
    auto row = p->children.indexOf( item );

    return createIndex( row, 0, item.data() );
}

int WidgetModel::rowCount( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return 1;

    auto item = static_cast<WidgetItem *>( parent.internalPointer() );
    Q_ASSERT( item != nullptr );

    return item->children.size();
}

QModelIndex WidgetModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !hasIndex( row, column, parent ) )
        return QModelIndex();

    if ( !parent.isValid() )
    {
        if ( row != 0 || column >= COLUMN_COUNT )
            return QModelIndex();

        return createIndex( 0, column, root.data() );
    }

    auto p = static_cast<WidgetItem *>( parent.internalPointer() );
    auto item = p->children.at( row );
    Q_ASSERT( item );

    return createIndex( row, column, item.data() );
}

QModelIndex WidgetModel::parent( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    auto item = static_cast<WidgetItem *>( index.internalPointer() );
    auto parentItem = item->parentItem;
    if ( parentItem.isNull() )
        return QModelIndex();
    
    auto parentOfParentItem = parentItem->parentItem;
    if ( parentOfParentItem.isNull() )
        return createIndex( 0, 0, root.data() );

    // Getting row for parent
    auto row = -1;
    for ( auto i = 0; i < parentOfParentItem->children.size(); i++ )
    {
        if ( parentOfParentItem->children.at( i ) == parentItem )
        {
            row = i;
            break;
        }
    }
    Q_ASSERT( row >= 0 );

    return createIndex( row, 0, parentItem.data() );
}

void WidgetModel::rebuildCache()
{
    cache.clear();
    rebuildCacheRecursive( cache, root );
}

void WidgetModel::rebuildCacheRecursive( ItemCache& cache, const QSharedPointer<WidgetItem>& item )
{
    cache[item->widget.data()] = item;
    for ( auto child : item->children )
        rebuildCacheRecursive( cache, child );
}
