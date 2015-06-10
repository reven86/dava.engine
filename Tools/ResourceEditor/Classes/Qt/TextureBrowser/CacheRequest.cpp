/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


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
