#include "EventModel.h"

#include <QMetaObject>
#include <QMetaEnum>


EventModel::EventModel( QObject* parent )
    : QStandardItemModel( parent )
{
    build( *this );
}

void EventModel::build( QStandardItemModel& model )
{
    model.clear();

    const auto& mo = QEvent::staticMetaObject;
    auto me = mo.enumerator( mo.indexOfEnumerator( "Type" ) );

    for ( auto i = 0; i < me.keyCount(); i++ )
    {
        const auto value = me.value( i );
        const QString text = me.key( i );

        auto item = new QStandardItem( text );
        item->setData( value, EVENT_TYPE );
        item->setCheckable( true );

        model.appendRow( item );
    }
}
