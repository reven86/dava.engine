#pragma once

#include "TArc/Controls/ColorPicker/Private/AbstractColorPicker.h"

#include <QPointer>

namespace DAVA
{
namespace TArc
{
class PaletteHSV;
class GradientSlider;
class ColorPickerHSV : public AbstractColorPicker
{
    Q_OBJECT

public:
    explicit ColorPickerHSV(QWidget* parent = NULL);

protected:
    void SetColorInternal(const QColor& c);

private slots:
    void OnChanging();
    void OnChanged();

    void OnHS();
    void OnVal();
    void OnAlpha();

private:
    void UpdateColor();

    QPointer<PaletteHSV> pal;
    QPointer<GradientSlider> val;
    QPointer<GradientSlider> alpha;
};
}
}
