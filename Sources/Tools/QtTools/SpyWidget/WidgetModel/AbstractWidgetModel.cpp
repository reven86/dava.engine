#include "AbstractWidgetModel.h"

#include <QMetaObject>


AbstractWidgetModel::AbstractWidgetModel( QObject* parent )
    : QAbstractItemModel( parent )
{
}

AbstractWidgetModel::~AbstractWidgetModel()
{
}

QWidget* AbstractWidgetModel::widgetFromIndex( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return nullptr;

    auto widget = static_cast<QWidget *>( index.internalPointer() );
    return widget;
}

int AbstractWidgetModel::columnCount( const QModelIndex& parent ) const
{
    return 3;
}

QVariant AbstractWidgetModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    return QVariant();
}

Qt::ItemFlags AbstractWidgetModel::flags( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant AbstractWidgetModel::data( const QModelIndex& index, int role ) const
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

    return QVariant();
}

bool AbstractWidgetModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    Q_UNUSED( index );
    Q_UNUSED( value );
    Q_UNUSED( role );

    return false;
}

QVariant AbstractWidgetModel::textDataForColumn( const QModelIndex& index ) const
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
