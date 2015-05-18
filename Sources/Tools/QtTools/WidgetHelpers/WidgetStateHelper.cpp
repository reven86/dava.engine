#include "WidgetStateHelper.h"

#include <QWidget>
#include <QEvent>
#include <QDebug>


WidgetStateHelper::WidgetStateHelper( QObject* parent )
    : QObject( parent )
{
    if ( parent != nullptr && parent->isWidgetType() )
    {
        startTrack( qobject_cast<QWidget *>( parent ) );
    }
}

WidgetStateHelper::~WidgetStateHelper()
{
    stopTrack();
}

void WidgetStateHelper::startTrack( QWidget* w )
{
    stopTrack();

    trackedWidget = w;
    if ( !trackedWidget.isNull() )
    {
        trackedWidget->installEventFilter( this );
        connect( trackedWidget.data(), &QObject::destroyed, this, &WidgetStateHelper::stopTrack );
    }
}

WidgetStateHelper::WidgetEvents WidgetStateHelper::getTrackedEvents() const
{
    return trackedEvents;
}

void WidgetStateHelper::setTrackedEvents( const WidgetEvents& events )
{
    trackedEvents = events;
}

bool WidgetStateHelper::eventFilter( QObject* watched, QEvent* event )
{
    if ( watched == trackedWidget )
    {
        switch ( event->type() )
        {
        case QEvent::Show:
            onShowEvent();
            break;

        default:
            break;
        }
    }

    return QObject::eventFilter( watched, event );
}

void WidgetStateHelper::stopTrack()
{
    if ( !trackedWidget.isNull() )
    {
        trackedWidget->removeEventFilter( this );
        trackedWidget = nullptr;
    }
}

void WidgetStateHelper::onShowEvent()
{
    if ( trackedEvents.testFlag( MaximizeOnShowOnce ) )
    {
        trackedWidget->showMaximized();
        trackedEvents &= ~MaximizeOnShowOnce;
    }
}

WidgetStateHelper* WidgetStateHelper::create( QWidget* w )
{
    WidgetStateHelper *helper = nullptr;

    if ( w != nullptr )
    {
        helper = w->findChild< WidgetStateHelper * >( QString(), Qt::FindDirectChildrenOnly );
    }

    if ( helper == nullptr )
    {
        helper = new WidgetStateHelper( w );
    }

    return helper;
}
