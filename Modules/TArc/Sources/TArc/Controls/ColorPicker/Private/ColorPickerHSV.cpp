#include "TArc/Controls/ColorPicker/Private/ColorPickerHSV.h"
#include "TArc/Controls/ColorPicker/Private/PaletteHSV.h"
#include "TArc/Controls/ColorPicker/Private/GradientSlider.h"

#include <QHBoxLayout>

namespace DAVA
{
namespace TArc
{
ColorPickerHSV::ColorPickerHSV(QWidget* parent)
    : AbstractColorPicker(parent)
{
    setObjectName("ColorPickerHSV");

    QHBoxLayout* l = new QHBoxLayout();

    pal = new PaletteHSV(this);
    pal->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    l->addWidget(pal);

    val = new GradientSlider(this);
    val->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    val->setMinimumSize(30, 0);
    val->SetDimensions(LEFT_EDGE | RIGHT_EDGE);
    val->SetOrientation(Qt::Vertical);
    val->SetOffsets(5, 5, 5, 5);
    l->addWidget(val);

    alpha = new GradientSlider(this);
    alpha->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    alpha->setMinimumSize(30, 0);
    alpha->SetDimensions(LEFT_EDGE | RIGHT_EDGE);
    alpha->SetOrientation(Qt::Vertical);
    alpha->SetOffsets(5, 5, 5, 5);
    l->addWidget(alpha);

    setLayout(l);
    updateGeometry();

    QObject* editors[] = { pal, val, alpha };

    for (size_t i = 0; i < sizeof(editors) / sizeof(*editors); i++)
    {
        connect(editors[i], SIGNAL(started(const QPointF&)), SIGNAL(begin()));
        connect(editors[i], SIGNAL(started(const QPointF&)), SLOT(OnChanging()));
        connect(editors[i], SIGNAL(changing(const QPointF&)), SLOT(OnChanging()));
        connect(editors[i], SIGNAL(changed(const QPointF&)), SLOT(OnChanged()));
        connect(editors[i], SIGNAL(canceled()), SIGNAL(canceled()));
    }

    connect(pal, SIGNAL(started(const QPointF&)), SLOT(OnHS()));
    connect(pal, SIGNAL(changing(const QPointF&)), SLOT(OnHS()));
    connect(val, SIGNAL(started(const QPointF&)), SLOT(OnVal()));
    connect(val, SIGNAL(changing(const QPointF&)), SLOT(OnVal()));
}

void ColorPickerHSV::SetColorInternal(QColor const& c)
{
    QColor min;
    QColor max;
    int h, s, v, a;
    c.getHsv(&h, &s, &v, &a);

    pal->SetColor(h, s);

    min.setHsv(h, s, 0, 255);
    max.setHsv(h, s, 255, 255);
    val->SetColors(max, min);
    const double dv = 1.0 - double(v) / 255.0;
    val->SetValue(dv);

    min.setHsv(h, s, v, 0);
    max.setHsv(h, s, v, 255);
    alpha->SetColors(max, min);
    const double da = 1.0 - double(a) / 255.0;
    alpha->SetValue(da);
}

void ColorPickerHSV::OnChanging()
{
    UpdateColor();
    emit changing(GetColor());
}

void ColorPickerHSV::OnChanged()
{
    UpdateColor();
    emit changed(GetColor());
}

void ColorPickerHSV::OnHS()
{
    int h, s, v, a;
    const QColor c = GetColor();
    QColor min;
    QColor max;

    c.getHsv(&h, &s, &v, &a);

    min.setHsv(h, s, 0, 255);
    max.setHsv(h, s, 255, 255);
    val->SetColors(max, min);

    min.setHsv(h, s, v, 0);
    max.setHsv(h, s, v, 255);
    alpha->SetColors(max, min);
}

void ColorPickerHSV::OnVal()
{
    int h, s, v, a;
    const QColor c = GetColor();
    QColor min;
    QColor max;

    c.getHsv(&h, &s, &v, &a);

    min.setHsv(h, s, v, 0);
    max.setHsv(h, s, v, 255);
    alpha->SetColors(max, min);
}

void ColorPickerHSV::OnAlpha()
{
}

void ColorPickerHSV::UpdateColor()
{
    qreal ho, so, vo, ao;
    color.getHsvF(&ho, &so, &vo, &ao);

    const qreal hn = pal->GetHue() / 360.0;
    const qreal sn = pal->GetSat() / 256.0;
    const qreal vn = 1.0 - val->GetValue();
    const qreal an = 1.0 - alpha->GetValue();

    qreal h = ho;
    qreal s = so;
    qreal v = vo;
    qreal a = ao;

    // We should update only changed values to reduce rounding
    if (sender() == pal)
    {
        h = hn;
        s = sn;
    }
    else if (sender() == val)
    {
        v = vn;
    }
    else if (sender() == alpha)
    {
        a = an;
    }

    QColor c;
    c.setHsvF(h, s, v, a);
    color = c;
}
}
}
