#include "Container.h"

#include <QDebug>
#include <QEvent>
#include <QVBoxLayout>

#include "Backend.h"


Container::Container(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy( Qt::StrongFocus );

    //setAttribute( Qt::WA_TranslucentBackground );
    //setAttribute( Qt::WA_NoSystemBackground );

    m_backend = new Backend(parent);
    m_backend->move( pos() );
    m_backend->resize( size() );
    m_backend->show();
    
    raise();
}

Container::~Container()
{
}

bool Container::event( QEvent* e )
{
    switch ( e->type() )
    {
    case QEvent::KeyPress:
        qDebug() << "QEvent::KeyPress";
        break;

    case QEvent::KeyRelease:
        qDebug() << "QEvent::KeyRelease";
        break;

    case QEvent::MouseButtonPress:
        qDebug() << "QEvent::MouseButtonPress";
        break;

    case QEvent::MouseButtonRelease:
        qDebug() << "QEvent::MouseButtonRelease";
        break;

    default:
        // qDebug() << __FUNCTION__;
        break;
    }

    return QWidget::event( e );
}

void Container::keyPressEvent( QKeyEvent* e )
{
    //qDebug() << __FUNCTION__;
    return QWidget::keyPressEvent( e );
}

void Container::keyReleaseEvent( QKeyEvent* e )
{
    //qDebug() << __FUNCTION__;
    return QWidget::keyReleaseEvent( e );
}

void Container::resizeEvent( QResizeEvent* e )
{
    if ( m_backend )
        m_backend->resize( size() );
}

void Container::moveEvent( QMoveEvent* e )
{
    if ( m_backend )
        m_backend->move( pos() );
}
