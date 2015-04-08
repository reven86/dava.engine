#include "SpyDragWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QMetaEnum>


SpyDragWidget::SpyDragWidget( QWidget* parent )
    : QLabel( parent )
    , pix( QPixmap( ":/QtTools/wand.png" ) )
{
    setPixmap( pix );
    cur = QCursor( pix, 9, 8 );

    //const auto& mo = QEvent::staticMetaObject;
    //auto me = mo.enumerator( mo.indexOfEnumerator( "Type" ) );
    //qDebug() << __FUNCTION__ << " " << me.valueToKey( e->type() );
}

SpyDragWidget::~SpyDragWidget()
{
}

void SpyDragWidget::mousePressEvent( QMouseEvent* e )
{
    onMousePress();
}

void SpyDragWidget::mouseReleaseEvent( QMouseEvent* e )
{
    onMouseRelease();
}

void SpyDragWidget::onMousePress()
{
    QApplication::setOverrideCursor( cur );
    setPixmap( QPixmap() );

    emit mousePressed();
}

void SpyDragWidget::onMouseRelease()
{
    QApplication::restoreOverrideCursor();
    setPixmap( pix );

    emit mouseReleased( QCursor::pos() );
}
