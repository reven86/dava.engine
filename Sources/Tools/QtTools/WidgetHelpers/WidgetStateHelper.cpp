#include "WidgetStateHelper.h"

#include <QWidget>
#include <QWindow>
#include <QScreen>

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
    auto window = trackedWidget->windowHandle();
    if ( window != nullptr )
    {
        disconnect( window, &QWindow::screenChanged, this, &WidgetStateHelper::onScreenChanged );
        connect( window, &QWindow::screenChanged, this, &WidgetStateHelper::onScreenChanged );
        prevScreen = window->screen();
    }

    if ( trackedEvents.testFlag( MaximizeOnShowOnce ) )
    {
        trackedWidget->showMaximized();
        trackedEvents &= ~MaximizeOnShowOnce;
    }
}

void WidgetStateHelper::onScreenChanged( QScreen* screen )
{
    if ( !trackedEvents.testFlag( ScaleOnDisplayChange ) )
        return;

    if ( trackedWidget->isMaximized() )
        return;

    const auto size = trackedWidget->size();

    if ( prevScreen.isNull() )
    {
        prevScreen = screen;
    }

    const auto dw = qreal( size.width() ) / prevScreen->geometry().width();
    const auto dh = qreal( size.height() ) / prevScreen->geometry().height();
    const auto w = screen->geometry().width() * dw;
    const auto h = screen->geometry().height() * dh;

    // Resize is not working, while window is moving
    //trackedWidget->resize( w, h );
    Q_UNUSED( w );
    Q_UNUSED( h );

    trackedWidget->setMaximumSize( screen->availableGeometry().size() );
    prevScreen = screen;
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
