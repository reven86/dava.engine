#include "WidgetListModel.h"

#include <QWidget>
#include <QMetaObject>


WidgetListModel::WidgetListModel( QObject* parent )
    : AbstractWidgetModel( parent )
{
}

WidgetListModel::~WidgetListModel()
{
}

QWidget* WidgetListModel::widgetFromIndex( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return nullptr;

    auto widget = static_cast<QWidget *>( index.internalPointer() );
    return widget;
}

int WidgetListModel::rowCount( const QModelIndex& parent ) const
{
    if ( !parent.isValid() )
        return widgets.size();

    return 0;
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

void WidgetListModel::setWidgetList( const QWidgetList& widgetList )
{
    beginResetModel();

    for ( auto w : widgets )
    {
        disconnect( w, nullptr, this, nullptr );
    }

    widgets = widgetList;
    widgets.removeAll( nullptr );
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
