#include "WidgetModel.h"

#include <QApplication>



WidgetModel::WidgetModel( QObject* parent )
    : QAbstractItemModel( parent )
    , root( nullptr )
{
    if ( parent != nullptr && parent->isWidgetType() )
    {
        trackWidget( qobject_cast<QWidget *>( parent ) );
    }
}

WidgetModel::~WidgetModel()
{
}

void WidgetModel::trackWidget( QWidget* w )
{
    beginResetModel();

    if ( !root.isNull() )
        root->removeEventFilter( this );

    root = w;

    if ( !root.isNull() )
        root->installEventFilter( this );

    endResetModel();
}

bool WidgetModel::eventFilter( QObject* obj, QEvent* e )
{
    return QAbstractItemModel::eventFilter( obj, e );
}

int WidgetModel::columnCount( const QModelIndex& parent ) const
{
    return COLUMN_COUNT;
}

int WidgetModel::rowCount( const QModelIndex& parent ) const
{
    if ( root.isNull() )
        return 0;

    if ( !parent.isValid() )
        return 1;

    auto w = static_cast<QWidget *>( parent.internalPointer() );
    auto children = w->findChildren< QWidget * >( QString(), Qt::FindDirectChildrenOnly );

    return children.size();
}

QVariant WidgetModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    return QVariant();
}

bool WidgetModel::canFetchMore( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return false;

    auto w = static_cast<QWidget *>( parent.internalPointer() );
    auto children = w->findChildren< QWidget * >( QString(), Qt::FindDirectChildrenOnly );

    return !children.isEmpty();
}

void WidgetModel::fetchMore( const QModelIndex& parent )
{
    auto w = static_cast<QWidget *>( parent.internalPointer() );
    auto children = w->findChildren< QWidget * >( QString(), Qt::FindDirectChildrenOnly );

    for ( auto child : children )
        child->installEventFilter( this );
}

bool WidgetModel::hasChildren( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return !root.isNull();

    auto w = static_cast<QWidget *>( parent.internalPointer() );
    auto children = w->findChildren< QWidget * >( QString(), Qt::FindDirectChildrenOnly );

    return !children.isEmpty();
}

QModelIndex WidgetModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !hasIndex( row, column, parent ) )
        return QModelIndex();

    if ( !parent.isValid() )
    {
        Q_ASSERT( row == 0 );
        return createIndex( row, column, root.data() );
    }

    auto pw = static_cast<QWidget *>( parent.internalPointer() );
    auto children = pw->findChildren< QWidget * >( QString(), Qt::FindDirectChildrenOnly );
    auto w = children.at( row );
    Q_ASSERT( w );

    return createIndex( row, column, w );
}

QModelIndex WidgetModel::parent( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    auto w = static_cast<QWidget *>( index.internalPointer() );
    auto pw = w->parentWidget();

    if ( pw == nullptr )
        return QModelIndex();

    if ( pw->parentWidget() == nullptr )
        return createIndex( 0, 0, root.data() );

    auto parentList = pw->parentWidget()->findChildren< QWidget * >( QString(), Qt::FindDirectChildrenOnly );
    auto parentRow = parentList.indexOf( pw );
    Q_ASSERT( parentRow >= 0 );

    return createIndex( parentRow, 0, pw );
}