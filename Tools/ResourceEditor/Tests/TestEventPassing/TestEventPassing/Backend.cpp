#include "Backend.h"

#include <QDebug>
#include <QEvent>
#include <QPainter>


Backend::Backend( QWidget *parent )
    : QWidget( parent )
{
    setAttribute( Qt::WA_NativeWindow );
    setFocusPolicy( Qt::StrongFocus );
}

Backend::~Backend()
{
}

bool Backend::event( QEvent* e )
{
    switch ( e->type() )
    {
    case QEvent::KeyPress:
        qDebug() << "Backend: QEvent::KeyPress";
        break;

    case QEvent::KeyRelease:
        qDebug() << "Backend: QEvent::KeyRelease";
        break;

    case QEvent::MouseButtonPress:
        qDebug() << "Backend: QEvent::MouseButtonPress";
        break;

    case QEvent::MouseButtonRelease:
        qDebug() << "Backend: QEvent::MouseButtonRelease";
        break;

    default:
        // qDebug() << __FUNCTION__;
        break;
    }

    return QWidget::event( e );
}

void Backend::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );

    QPainter p( this );
    p.fillRect( 0, 0, width(), height(), Qt::red );
}

void Backend::keyPressEvent( QKeyEvent* e )
{
    //qDebug() << __FUNCTION__;
    return QWidget::keyPressEvent( e );
}

void Backend::keyReleaseEvent( QKeyEvent* e )
{
    //qDebug() << __FUNCTION__;
    return QWidget::keyReleaseEvent( e );
}
