#include "ColorCell.h"

#include <QPainter>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>

#include "../Helpers/MouseHelper.h"
#include "../Helpers/PaintingHelper.h"


ColorCell::ColorCell(QWidget* parent)
    : QWidget(parent)
    , color(Qt::transparent)
    , isHovered(false)
    , bgBrush(PaintingHelper::DrawGridBrush(QSize(5, 5)))
    , mouse(new MouseHelper(this))
{
    setMouseTracking(true);
    setAcceptDrops(true);

    connect(mouse, SIGNAL( clicked() ), SLOT( OnMouseClick() ));
    connect(mouse, SIGNAL( mouseEntered() ), SLOT( OnMouseEnter() ));
    connect(mouse, SIGNAL( mouseLeaved() ), SLOT( OnMouseLeave() ));
}

ColorCell::~ColorCell()
{
}

QColor const& ColorCell::GetColor() const
{
    return color;
}

void ColorCell::SetColor(QColor const& _color)
{
    if (color != _color)
    {
        color = _color;
        update();
    }
}

void ColorCell::OnMouseEnter()
{
    isHovered = true;
    update();
}

void ColorCell::OnMouseLeave()
{
    isHovered = false;
    update();
}

void ColorCell::OnMouseClick()
{
    emit clicked(GetColor());
}

void ColorCell::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);

    p.fillRect(rect(), bgBrush);
    p.fillRect(rect(), GetColor());

    p.setPen(Qt::black);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
    if (isHovered)
    {
        p.drawRect(rect().adjusted(1, 1, -2, -2 ));
    }
}

void ColorCell::dragEnterEvent(QDragEnterEvent* e)
{
    const QMimeData *mime = e->mimeData();
    if (mime->hasColor())
    {
        e->acceptProposedAction();
        OnMouseEnter();
    }
}

void ColorCell::dragLeaveEvent(QDragLeaveEvent* e)
{
    Q_UNUSED(e);
    OnMouseLeave();
}

void ColorCell::dragMoveEvent(QDragMoveEvent* e)
{
    const QMimeData *mime = e->mimeData();
    if (mime->hasColor())
    {
        e->acceptProposedAction();
    }
}

void ColorCell::dropEvent(QDropEvent* e)
{
    const QMimeData *mime = e->mimeData();
    if (mime->hasColor())
    {
        const QColor& c = qvariant_cast<QColor>(mime->colorData());
        SetColor(c);
        e->acceptProposedAction();
    }
}
