#pragma once

#include "TArc/Controls/ColorPicker/Private/AbstractSlider.h"

namespace DAVA
{
namespace TArc
{
class PaletteHSV : public AbstractSlider
{
    Q_OBJECT

public:
    explicit PaletteHSV(QWidget* parent);

    int GetHue() const;
    int GetSat() const;

    void SetColor(int hue, int sat);
    void SetColor(const QColor& c);

protected:
    void DrawBackground(QPainter* p) const;
    void DrawForeground(QPainter* p) const;
    QRect PosArea() const;

    void resizeEvent(QResizeEvent* e);

private:
    void DrawCursor(QPainter* p) const;

    QSize cursorSize;
    mutable QPixmap bgCache;
};
}
}
