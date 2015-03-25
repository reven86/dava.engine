#include "AbstractWidgetModel.h"

#include <QMetaObject>


AbstractWidgetModel::AbstractWidgetModel( QObject* parent )
    : QAbstractItemModel( parent )
{
}

AbstractWidgetModel::~AbstractWidgetModel()
{
}

int AbstractWidgetModel::columnCount( const QModelIndex& parent ) const
{
    return COLUMN_COUNT;
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

    auto w = widgetFromIndex( index );
    if ( w == nullptr )
        return QVariant();

    switch ( role )
    {
    case Qt::DisplayRole:
        return textDataForColumn( w, index.column() );
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

QVariant AbstractWidgetModel::textDataForColumn( QWidget *w, int column ) const
{
    auto meta = w->metaObject();

    switch ( column )
    {
    case TITLE:
        return QString( "%1" ).arg( meta->className() );
    case CLASSNAME:
        return QString( ".%1" ).arg( meta->className() );
    case OBJECTNAME:
        return QString( "#%1" ).arg( w->objectName() );
    default:
        break;
    }

    return QVariant();
}
