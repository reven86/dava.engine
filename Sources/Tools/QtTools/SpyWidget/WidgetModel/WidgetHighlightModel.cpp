#include "WidgetHighlightModel.h"


WidgetHighlightModel::WidgetHighlightModel( QObject* parent )
    : QIdentityProxyModel( parent )
{
}

WidgetHighlightModel::~WidgetHighlightModel()
{
}

QVariant WidgetHighlightModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    auto realIndex = mapToSource( index );
    auto w = static_cast<QWidget *>( realIndex.internalPointer() );

    switch ( role )
    {
    case Qt::BackgroundRole:
        if ( widgets.contains( w ) )
        {
            return QColor( 0, 255, 0, 150 );
        }
        break;
    default:
        break;
    }

    return realIndex.data( role );
}

void WidgetHighlightModel::setWidgetList( const QSet< QWidget * >& widgetsToHighlight )
{
    for ( auto w : widgets )
    {
        disconnect( w, nullptr, this, nullptr );
    }

    widgets = widgetsToHighlight;
    for ( auto w : widgets )
    {
        connect( w, &QObject::destroyed, this, &WidgetHighlightModel::onWidgetDestroyed );
    }
    invalidate();
}

void WidgetHighlightModel::onWidgetDestroyed()
{
    auto w = qobject_cast<QWidget *>( sender() );
    widgets.remove( w );
    invalidate();
}

void WidgetHighlightModel::invalidate()
{
    static const auto roles = QVector<int>() << Qt::DisplayRole << Qt::TextColorRole << Qt::BackgroundRole;
    emit dataChanged( QModelIndex(), QModelIndex(), roles );
}
