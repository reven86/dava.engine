#include "UI/Preview/Guides/GuideLabel.h"

#include <QPainter>

GuideLabel::GuideLabel(DAVA::Vector2::eAxis orientation_, QWidget* parent)
    : QWidget(parent)
    , orientation(orientation_)
{
    setAttribute(Qt::WA_TranslucentBackground); // Indicates that the background will be transparent
}

void GuideLabel::SetValue(int arg)
{
    value = arg;
}

void GuideLabel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QPalette palette;
    QColor rectColor = palette.color(QPalette::Window);
    rectColor = rectColor.lighter();
    rectColor.setAlpha(220);

    painter.save();
    painter.setBrush(QBrush(rectColor));
    painter.setPen(rectColor);

    const int radius = 0;
    QRect roundedRect(radius / 2,
                      radius / 2,
                      width() - radius,
                      height() - radius);
    painter.drawRoundedRect(roundedRect, radius, radius);

    painter.restore();

    QRect textRect = rect();
    if (orientation == DAVA::Vector2::AXIS_Y)
    {
        painter.translate(0, height());
        painter.rotate(-90);
        textRect.setWidth(height());
        textRect.setHeight(width());
    }
    painter.drawText(textRect, Qt::AlignCenter, QString::number(value));
}
