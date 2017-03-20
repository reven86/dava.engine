#pragma once

#include "TArc/Controls/ColorPicker/Private/AbstractColorPicker.h"

#include <QPointer>

namespace DAVA
{
namespace TArc
{
class ColorComponentSlider;
class ColorPickerRGBAM : public AbstractColorPicker
{
    Q_OBJECT

public:
    explicit ColorPickerRGBAM(QWidget* parent = NULL);

    double GetMultiplierValue() const;
    void SetMultiplierValue(double val);

    double GetMaxMultiplierValue() const;
    void SetMaxMultiplierValue(double val);

private slots:
    void OnChanging(double val);
    void OnChanged(double val);

private:
    void SetColorInternal(QColor const& c);
    void UpdateColorInternal(ColorComponentSlider* source = NULL);

    QLayout* CreateSlider(const QString& text, ColorComponentSlider* w) const;

    QPointer<ColorComponentSlider> r;
    QPointer<ColorComponentSlider> g;
    QPointer<ColorComponentSlider> b;
    QPointer<ColorComponentSlider> a;
    QPointer<ColorComponentSlider> m;
};
}
}
