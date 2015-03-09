#include "AbstractColorPicker.h"


AbstractColorPicker::AbstractColorPicker(QWidget* parent)
    : QWidget(parent)
{
}

AbstractColorPicker::~AbstractColorPicker()
{
}

const QColor& AbstractColorPicker::GetColor() const
{
    return color;
}

void AbstractColorPicker::SetColor(const QColor& c)
{
    color = c;
    SetColorInternal(c);
}