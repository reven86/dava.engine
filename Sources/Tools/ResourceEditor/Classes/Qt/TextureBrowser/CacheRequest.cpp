#include "CacheRequest.h"

#include <QMetaObject>
#include <string>
#include <QList>
#include <QDebug>


CacheRequest::CacheRequest( const DAVA::FilePath _key )
	: QObject( NULL )
	, key( _key )
{
}

CacheRequest::~CacheRequest()
{
}

void CacheRequest::registerObserver( QObject *object, const QString& slot, const QVariant& userData )
{
	Q_ASSERT( object );

	connect( object, SIGNAL( destroyed() ), SLOT( onObserverDestroyed() ) );
	observers[object] << SlotWithArg( slot, userData );
}

void CacheRequest::invoke( QList< QImage > images )
{
    for ( ObserverMap::iterator i = observers.begin(); i != observers.end(); i++ )
	{
		QObject *obj = i.key();
		const SlotList& slotList = i.value();
		foreach ( const SlotWithArg& slotInfo, slotList )
		{
			const std::string methodName = slotInfo.first.toStdString();
			const QVariant arg1 = slotInfo.second;

			QMetaObject::invokeMethod( obj, methodName.c_str(), Qt::QueuedConnection, Q_ARG( QList< QImage >, images ), Q_ARG( QVariant, arg1 ) );
		}
	}
}

void CacheRequest::onObserverDestroyed()
{
	observers.remove( sender() );
}
