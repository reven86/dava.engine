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


#include "DavaLoop.h"

#include <QElapsedTimer>
#include <QTimer>

#include "Debug/DVAssert.h"



LoopItem::LoopItem( QObject* parent )
    : QObject( parent )
    , maxFps( 0 )
    , fps( 0 )
    , paused(false)
{
}

LoopItem::~LoopItem()
{
}

void LoopItem::SetMaxFps( int _maxFps )
{
    DVASSERT( _maxFps > 0 );
    maxFps = _maxFps;
}

int LoopItem::MaxFps() const
{
    return maxFps;
}

int LoopItem::Fps() const
{
    return fps;
}

void LoopItem::ProcessFrame()
{
    if(!paused)
    {
        ProcessFrameInternal();
    }
}

void LoopItem::ProcessFrameInternal()
{
    
}

void LoopItem::SetPaused(bool _paused)
{
    paused = _paused;
}

bool LoopItem::GetPaused() const
{
    return paused;
}



DavaLoop::DavaLoop()
    : QObject( nullptr )
{
}

DavaLoop::~DavaLoop()
{
    for ( auto it = loops.begin(); it != loops.end(); ++it )
    {
        QTimer *t = it.key();
        LoopItem *item = it.value();

        disconnect( item, &QObject::destroyed, this, &DavaLoop::OnItemDestroyed );
        delete t;
        if ( item->parent() == nullptr )
            delete item;
    }

    loops.clear();
}

void DavaLoop::StartLoop( LoopItem* item )
{
    DVASSERT( item != nullptr );

    QTimer *t = new QTimer( this );
    t->setSingleShot( true );

    loops[t] = item;

    connect( item, &QObject::destroyed, this, &DavaLoop::OnItemDestroyed );
    connect( t, &QTimer::timeout, this, &DavaLoop::ProcessItem );

    t->start( 0 );
}

void DavaLoop::StopLoop( LoopItem* item )
{
    QList< QTimer * > timers = loops.keys( item );
    for ( int i = 0; i < timers.size(); i++ )
    {
        QTimer *t = timers[i];
        t->stop();
        loops.remove( t );
        delete t;
    }
}

void DavaLoop::ProcessItem()
{
    QTimer *t = qobject_cast< QTimer * >( sender() );
    
    auto it = loops.find( t );
    if ( it == loops.end() || t == nullptr )
        return;

    QPointer< LoopItem > item = it.value();
    if ( item == nullptr )
        return;

    QElapsedTimer frameTimer;
    frameTimer.start();

    item->ProcessFrame();

    qint64 waitUntilNextFrameMs = 1000 / item->MaxFps() - frameTimer.elapsed();
    if ( waitUntilNextFrameMs < 0 )
    {
        waitUntilNextFrameMs = 0;
    }

    t->start( waitUntilNextFrameMs );
}

void DavaLoop::OnItemDestroyed()
{
    QPointer< LoopItem > item = qobject_cast<LoopItem *>( sender() );
    StopLoop( item );
}
