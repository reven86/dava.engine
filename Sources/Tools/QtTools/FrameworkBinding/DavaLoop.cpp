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
