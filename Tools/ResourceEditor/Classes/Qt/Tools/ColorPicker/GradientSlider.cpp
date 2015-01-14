#include "GradientSlider.h"

#include <QPainter>

#include "../Helpers/PaintingHelper.h"


GradientSlider::GradientSlider(QWidget* parent)
    : AbstractSlider(parent)
    , arrowSize(9, 9)
    , arrows(0)
    , orientation(Qt::Horizontal)
    , ofsL(0)
    , ofsR(0)
    , ofsT(0)
    , ofsB(0)
    , bgBrush(PaintingHelper::DrawGridBrush(QSize(5, 5)))
{
}

GradientSlider::~GradientSlider()
{
}

void GradientSlider::SetColors(const QColor& _c1, const QColor& _c2)
{
    c1 = _c1;
    c2 = _c2;
    bgCache = QPixmap();
    update();
}

void GradientSlider::SetDimensions(const EDGES& flags)
{
    arrows = flags;
    bgCache = QPixmap();
    update();
}

void GradientSlider::SetOrientation(Qt::Orientation _orientation)
{
    orientation = _orientation;
    update();
}

void GradientSlider::SetOffsets(int l, int t, int r, int b)
{
    ofsL = l;
    ofsT = t;
    ofsR = r;
    ofsB = b;
    SetPosF(PosF()); // recalculate cursor coordinates
}

double GradientSlider::GetValue() const
{
    return (orientation == Qt::Horizontal) ? PosF().x() : PosF().y();
}

void GradientSlider::SetValue(double val)
{
    switch (orientation)
    {
    case Qt::Horizontal:
        SetPosF(QPointF(val, PosF().y()));
        break;
    case Qt::Vertical:
        SetPosF(QPointF(PosF().x(), val));
        break;
    }
}

void GradientSlider::DrawBackground(QPainter* p) const
{
    const QRect& rc = PosArea();

    if (bgCache.isNull())
    {
        const QImage& bg = PaintingHelper::DrawGradient(rc.size(), c1, c2, orientation);
        bgCache = QPixmap::fromImage(bg);
    }

    p->fillRect(rc, bgBrush);
    p->drawPixmap(rc.topLeft(), bgCache);
}

void GradientSlider::DrawForeground(QPainter* p) const
{
    EDGES flags;
    switch (orientation)
    {
    case Qt::Horizontal:
        flags = arrows & (TOP_EDGE | BOTTOM_EDGE);
        break;
    case Qt::Vertical:
        flags = arrows & (LEFT_EDGE | RIGHT_EDGE);
        break;
    }

    p->setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing, false );

    if (flags.testFlag(TOP_EDGE))
        drawArrow(TOP_EDGE, p);
    if (flags.testFlag(LEFT_EDGE))
        drawArrow(LEFT_EDGE, p);
    if (flags.testFlag(RIGHT_EDGE))
        drawArrow(RIGHT_EDGE, p);
    if (flags.testFlag(BOTTOM_EDGE))
        drawArrow(BOTTOM_EDGE, p);

    // Debug draw
    //p->setPen( Qt::red );
    //p->drawRect( rect().adjusted( 0, 0, -1, -1 ) );
    //p->setPen( Qt::green );
    //p->drawRect( PosArea() );
}

QRect GradientSlider::PosArea() const
{
    return rect().adjusted(ofsL, ofsT, -ofsR - 1, -ofsB - 1);
}

void GradientSlider::resizeEvent(QResizeEvent* e)
{
    bgCache = QPixmap();
    AbstractSlider::resizeEvent(e);
}

void GradientSlider::drawArrow(EDGE arrow, QPainter* p) const
{
    const auto it = arrowCache.constFind(arrow);
    if (it == arrowCache.constEnd())
    {
        arrowCache[arrow] = QPixmap::fromImage(PaintingHelper::DrawArrowIcon(arrowSize, arrow, Qt::black));
    }

    const QPoint& currentPos = Pos();
    QPoint pos;

    switch (arrow)
    {
    case TOP_EDGE:
        pos.setX(currentPos.x() - arrowSize.width() / 2);
        pos.setY(0);
        break;
    case LEFT_EDGE:
        pos.setX(0);
        pos.setY(currentPos.y() - arrowSize.height() / 2);
        break;
    case RIGHT_EDGE:
        pos.setX(width() - arrowSize.width() + 1);
        pos.setY(currentPos.y() - arrowSize.height() / 2);
        break;
    case BOTTOM_EDGE:
        pos.setX(currentPos.x() - arrowSize.width() / 2);
        pos.setY(height() - arrowSize.height() + 1);
        break;
    default:
        return;
    }

    QRect rc(pos, arrowSize);
    p->drawPixmap(pos, arrowCache[arrow]);
}