#include "SpyWidgetInfo.h"

#include <QWidget>
#include <QMetaObject>
#include <QEvent>
#include <QTimer>

#include "SpyWidget.h"
#include "QtTools/SpyWidget/WidgetModel/WidgetModel.h"


namespace
{
    const int updateDelay = 100; // set to 0, if you don't care about performace and need real-time, event-driven updates
}


SpyWidgetInfo::SpyWidgetInfo( QObject* parent )
    : QObject( parent )
    , view( new SpyWidget() )
    , updateTimer( new QTimer(this) )
    , widgetModel( nullptr )
{
    view->setAttribute( Qt::WA_DeleteOnClose );

    updateTimer->setInterval( updateDelay );    
    updateTimer->setSingleShot( true );

    connect( view.data(), &QObject::destroyed, this, &QObject::deleteLater );
    connect( updateTimer.data(), &QTimer::timeout, this, &SpyWidgetInfo::updateInformation );
}

SpyWidgetInfo::~SpyWidgetInfo()
{
    delete view;
}

void SpyWidgetInfo::trackWidget( QWidget* w )
{
    if ( !widget.isNull() )
    {
        widget->removeEventFilter( this );
        disconnect( widget.data(), nullptr, this, nullptr );
        delete widgetModel;
    }

    widget = w;

    auto rootWidget = w->window();
    widgetModel = new WidgetModel( rootWidget );
    view->hierarhyTree->setModel( widgetModel );

    if ( !widget.isNull() )
    {
        widget->installEventFilter( this );
        connect( widget.data(), &QObject::objectNameChanged, updateTimer.data(), static_cast< void ( QTimer::* )( )>( &QTimer::start ) );
    }

    updateInformation();
}

bool SpyWidgetInfo::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == widget )
    {

        // Modality is tracked throught show/hide: http://doc.qt.io/qt-5/qwidget.html#windowModality-prop

        switch ( e->type() )
        {
        case QEvent::Show:
        case QEvent::Hide:
        case QEvent::MouseTrackingChange:
        case QEvent::Move:
        case QEvent::Resize:
            updateTimer->start();
            break;

#ifdef Q_OS_MAC
        case QEvent::MacSizeChange:
            updateTimer->start();
            break;
#endif

        default:
            break;
        }
    }

    return QObject::eventFilter( obj, e );
}

void SpyWidgetInfo::show()
{
    view->show();
}

void SpyWidgetInfo::updateInformation()
{
    if ( widget.isNull() )
    {
        auto textEditors = { view->classNameText, view->objectNameText, view->objectPos, view->objectSize, };
        for ( auto w : textEditors )
        {
            w->setText( QString() );
        }

        auto checkBoxes = { view->visibleState, view->modalState, view->mouseTrackState, };
        for ( auto w : checkBoxes )
        {
            w->setChecked( false );
        }

        return;
    }

    const auto mo = widget->metaObject();

    const auto classNameText = mo->className();
    const auto objectNameText = widget->objectName();
    const auto positionText = QString( "QPoint( %1, %2 )" ).arg( widget->x() ).arg( widget->y() );
    const auto sizeText = QString( "QSize( %1, %2 )" ).arg( widget->width() ).arg( widget->height() );
    const auto isVisible = widget->isVisible();
    const auto isModal = widget->isModal();
    const auto isMouseTracked = widget->hasMouseTracking();

    view->classNameText->setText( classNameText );
    view->objectNameText->setText( objectNameText );
    view->objectPos->setText( positionText );
    view->objectSize->setText( sizeText );
    view->visibleState->setChecked( isVisible );
    view->modalState->setChecked( isModal );
    view->mouseTrackState->setChecked( isMouseTracked );
}
