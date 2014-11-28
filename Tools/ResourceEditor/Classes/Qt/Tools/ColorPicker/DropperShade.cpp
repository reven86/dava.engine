#include "DropperShade.h"

#include <QPainter>
#include <QKeyEvent>
#include <QCursor>
#include <QLabel>

#include "../Helpers/MouseHelper.h"


DropperShade::DropperShade( const QPixmap& src, const QRect& rect )
: QWidget(NULL, Qt::Window | Qt::FramelessWindowHint/* | Qt::WindowStaysOnTopHint | Qt::ToolTip*/)
    , pixmap( src )
    , image( src.toImage() )
    , mouse(new MouseHelper(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);
    setCursor(Qt::BlankCursor);
    setFixedSize(rect.size());
    move(rect.topLeft());

    connect(mouse, &MouseHelper::mouseMove, this, &DropperShade::OnMouseMove);
    connect(mouse, &MouseHelper::mouseRelease, this, &DropperShade::OnClicked);
    connect(mouse, &MouseHelper::mouseWheel, this, &DropperShade::OnMouseWheel);
}

DropperShade::~DropperShade()
{
}

void DropperShade::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);
}

QColor DropperShade::GetPixel(const QPoint& pos) const
{
    const QColor c = image.pixel(pos);
    return c;
}

void DropperShade::OnMouseMove(const QPoint& pos)
{
    emit moved(mapToGlobal(pos));
    emit changed(GetPixel(pos));
}

void DropperShade::OnClicked(const QPoint& pos)
{
    emit picked(GetPixel(pos));
}

void DropperShade::OnMouseWheel(int delta)
{
    emit zoomFactorChanged(delta);
}

void DropperShade::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        emit canceled();
    }

    QWidget::keyPressEvent(e);
}
