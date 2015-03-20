#include "WidgetListModel.h"

#include <QWidget>
#include <QMetaObject>


WidgetListModel::WidgetListModel( QObject* parent )
    : QAbstractListModel( parent )
{
}

WidgetListModel::~WidgetListModel()
{
}

int WidgetListModel::columnCount( const QModelIndex& parent ) const
{
    return 3;
}

int WidgetListModel::rowCount( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return widgets.size();

    return 0;
}

QVariant WidgetListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    return QVariant();
}

QModelIndex WidgetListModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !hasIndex( row, column, parent ) )
        return QModelIndex();

    return createIndex( row, column, widgets[row] );
}

QModelIndex WidgetListModel::parent( const QModelIndex& index ) const
{
    return QModelIndex();
}

Qt::ItemFlags WidgetListModel::flags( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant WidgetListModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    auto widget = static_cast<QWidget *>( index.internalPointer() );
    switch ( role )
    {
    case Qt::DisplayRole:
        return textDataForColumn( index );
    default:
        break;
    }

    return QAbstractItemModel::data( index, role );
}

bool WidgetListModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    Q_UNUSED( index );
    Q_UNUSED( value );
    Q_UNUSED( role );

    return false;
}

void WidgetListModel::setWidgetList( const QWidgetList& widgetList )
{
    beginResetModel();

    for ( auto w : widgets )
    {
        disconnect( w, nullptr, this, nullptr );
    }

    widgets = widgetList;
    for ( auto w : widgets )
    {
        connect( w, &QObject::destroyed, this, &WidgetListModel::onWidgetDestroyed );
    }

    endResetModel();
}

void WidgetListModel::onWidgetDestroyed()
{
    auto w = qobject_cast<QWidget *>( sender() );
    const auto row = widgets.indexOf( w );
    Q_ASSERT( row >= 0 );

    beginRemoveRows( QModelIndex(), row, row );
    widgets.removeAt( row );
    endRemoveRows();
}

QVariant WidgetListModel::textDataForColumn( const QModelIndex& index ) const
{
    auto widget = static_cast<QWidget *>( index.internalPointer() );
    auto meta = widget->metaObject();

    switch ( index.column() )
    {
    case TITLE:
        return QString( "%1" ).arg( meta->className() );
    case CLASSNAME:
        return QString( ".%1" ).arg( meta->className() );
    case OBJECTNAME:
        return QString( "#%1" ).arg( widget->objectName() );
    default:
        break;
    }

    return QVariant();
}