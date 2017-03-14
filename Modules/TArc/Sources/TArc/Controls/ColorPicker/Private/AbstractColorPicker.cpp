#include "TArc/Controls/ColorPicker/Private/AbstractColorPicker.h"

namespace DAVA
{
namespace TArc
{
AbstractColorPicker::AbstractColorPicker(QWidget* parent)
    : QWidget(parent)
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
}
}
