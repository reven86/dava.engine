#include "DropperShade.h"

#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QCursor>
#include <QLabel>

#include "../Helpers/MouseHelper.h"

#include "DropperLens.h"


DropperShade::DropperShade( const QPixmap& src, const QRect& rect, DropperLens* _lens )
: QWidget(NULL, Qt::Window | Qt::FramelessWindowHint/* | Qt::WindowStaysOnTopHint | Qt::ToolTip*/)
    , pixmap(src)
    , image(src.toImage())
    , mouse(new MouseHelper(this))
    , lens(_lens)
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

    label = new QLabel(this);
    label->setStyleSheet( "background: white;" );
    label->setFixedSize(200, 30);
    label->move(0, 0);
}

DropperShade::~DropperShade()
{
}

void DropperShade::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.drawPixmap(e->rect(),pixmap,e->rect());
}

QColor DropperShade::GetPixel(const QPoint& pos) const
{
    const QColor c = image.pixel(pos);
    return c;
}

void DropperShade::updateLens()
{
    const int scale = static_cast<int>(pixmap.devicePixelRatio());
    const int zoom = lens->ZoomFactor() * 2 + 1;
    const int ptX = mouse->Pos().x() * scale;
    const int ptY = mouse->Pos().y() * scale;
    const int w = lens->width() * scale / zoom;
    const int h = lens->height() * scale / zoom;
    const int x = ptX - w / 2;
    const int y = ptY - h / 2;

    const QPixmap& crop = pixmap.copy(x, y, w, h).scaled( w * zoom, h * zoom );
    lens->preview() = crop;

    const QString text = QString( "%1 x %2" ).arg( ptX ).arg( ptY );
    label->setText(text);
}

void DropperShade::OnMouseMove(const QPoint& pos)
{
    updateLens();

    emit moved(mapToGlobal(pos));
    emit changed(GetPixel(pos));
}

void DropperShade::OnClicked(const QPoint& pos)
{
    emit picked(GetPixel(pos));
}

void DropperShade::OnMouseWheel(int delta)
{
    lens->changeZoom(delta);
    updateLens();
    lens->update();
}

void DropperShade::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        emit canceled();
    }

    QWidget::keyPressEvent(e);
}
