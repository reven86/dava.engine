#include "TArc/Controls/ColorPicker/ColorPickerDialog.h"
#include "TArc/Controls/ColorPicker/Private/AbstractColorPicker.h"
#include "TArc/Controls/ColorPicker/Private/ColorPickerHSV.h"
#include "TArc/Controls/ColorPicker/Private/ColorPickerRGBAM.h"
#include "TArc/Controls/ColorPicker/Private/ColorPreview.h"
#include "TArc/Controls/ColorPicker/Private/EyeDropper.h"

#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/PropertiesHolder.h"

#include "ui_ColorPicker.h"

#include <QtTools/Utils/Utils.h>

#include <Math/Color.h>

#include <Qt>
#include <QKeyEvent>
#include <QByteArray>
#include <QDataStream>

namespace DAVA
{
namespace TArc
{
namespace ColorPickerDialogDetail
{
const String PROPERTIES_KEY = "ColorPickerDialogProperties";
const String GEOMETRY_KEY = "CPD_geometry";
const String STATE_KEY = "CPD_state";
const String PALETTE_KEY = "CPD_palette";
const String MULTIPLIER_KEY = "CPD_maxMultiplier";
}

ColorPickerDialog::ColorPickerDialog(ContextAccessor* accessor, QWidget* parent)
    : AbstractColorPicker(parent)
    , ui(new Ui::ColorPickerDialog())
    , contextAccessor(accessor)
{
    ui->setupUi(this);

    setWindowFlags(static_cast<Qt::WindowFlags>(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint));
    setFocusPolicy(Qt::ClickFocus);
    setFixedSize(size());

    // Pickers
    RegisterPicker("HSV rectangle", new ColorPickerHSV());

    // Editors
    rgbam = new ColorPickerRGBAM();
    RegisterColorSpace("RGBA M", rgbam);

    // Preview
    connect(this, SIGNAL(changing(const QColor&)), ui->preview, SLOT(SetColorNew(const QColor&)));
    connect(this, SIGNAL(changed(const QColor&)), ui->preview, SLOT(SetColorNew(const QColor&)));

    // Dropper
    connect(ui->dropper, SIGNAL(clicked()), SLOT(OnDropper()));

    // Color picker
    connect(ui->ok, SIGNAL(clicked()), SLOT(OnOk()));
    connect(ui->cancel, SIGNAL(clicked()), SLOT(close()));

    // Custom palette
    connect(ui->customPalette, SIGNAL(selected(const QColor&)), SLOT(OnChanged(const QColor&)));

    SetColor(Qt::white);

    LoadSettings();
}

ColorPickerDialog::~ColorPickerDialog()
{
    SaveSettings();
}

bool ColorPickerDialog::Exec(const QString& title)
{
    const Qt::WindowFlags f = windowFlags();
    const Qt::WindowModality m = windowModality();
    setWindowFlags(f | Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setWindowOpacity(1.0);
    if (!title.isEmpty())
    {
        setWindowTitle(title);
    }

    show();
    modalLoop.exec();

    setWindowFlags(f);
    setWindowModality(m);

    return confirmed;
}

double ColorPickerDialog::GetMultiplierValue() const
{
    if (rgbam)
    {
        return rgbam->GetMultiplierValue();
    }

    return 0.0;
}

void ColorPickerDialog::SetMultiplierValue(double val)
{
    if (rgbam)
    {
        rgbam->SetMultiplierValue(val);
    }
}

void ColorPickerDialog::SetDavaColor(const Color& color)
{
    const QColor c = ColorToQColor(color);
    const double mul = CalculateMultiplier(color.r, color.g, color.b);

    SetColor(c);
    if (mul > 1.0)
    {
        SetMultiplierValue(mul);
    }
}

Color ColorPickerDialog::GetDavaColor() const
{
    const QColor c = GetColor();
    Color newColor = QColorToColor(c);
    const double mul = GetMultiplierValue();
    ApplyMultiplier(newColor.r, newColor.g, newColor.b, mul);

    return newColor;
}

double ColorPickerDialog::CalculateMultiplier(float r, float g, float b)
{
    const double components[] = { r, g, b };
    const size_t n = sizeof(components) / sizeof(*components);
    size_t iMax = 0;
    for (int i = 1; i < n; i++)
    {
        if (components[i] > components[iMax])
        {
            iMax = i;
        }
    }

    const double multiplier = qMax(components[iMax], 1.0);
    return multiplier;
}

bool ColorPickerDialog::RemoveMultiplier(float& r, float& g, float& b)
{
    const double multiplier = CalculateMultiplier(r, g, b);
    if (multiplier > 1.0)
    {
        r /= multiplier;
        g /= multiplier;
        b /= multiplier;
        return true;
    }

    return false;
}

void ColorPickerDialog::ApplyMultiplier(float& r, float& g, float& b, double mul)
{
    r *= mul;
    g *= mul;
    b *= mul;
}

void ColorPickerDialog::RegisterPicker(QString const& key, AbstractColorPicker* picker)
{
    delete pickers[key];
    pickers[key] = picker;

    ui->pickerCombo->addItem(key, key);
    ui->pickerStack->addWidget(picker);
    ConnectPicker(picker);
}

void ColorPickerDialog::RegisterColorSpace(const QString& key, AbstractColorPicker* picker)
{
    delete colorSpaces[key];
    colorSpaces[key] = picker;

    ui->colorSpaceCombo->addItem(key, key);
    ui->colorSpaceStack->addWidget(picker);
    ConnectPicker(picker);
}

void ColorPickerDialog::SetColorInternal(const QColor& c)
{
    UpdateControls(c);
    oldColor = c;
    ui->preview->SetColorOld(c);
    ui->preview->SetColorNew(c);
}

void ColorPickerDialog::OnChanging(const QColor& c)
{
    AbstractColorPicker* source = qobject_cast<AbstractColorPicker*>(sender());
    UpdateControls(c, source);
    emit changing(c);
}

void ColorPickerDialog::OnChanged(const QColor& c)
{
    AbstractColorPicker* source = qobject_cast<AbstractColorPicker*>(sender());
    UpdateControls(c, source);
    emit changed(c);
}

void ColorPickerDialog::OnDropperChanged(const QColor& c)
{
    QColor normalized(c);
    normalized.setAlphaF(GetColor().alphaF());
    UpdateControls(normalized);
    ui->preview->SetColorNew(normalized);
    emit changed(GetColor());
}

void ColorPickerDialog::OnDropper()
{
    dropper = new EyeDropper(this);
    connect(dropper, SIGNAL(picked(const QColor&)), SLOT(OnDropperChanged(const QColor&)));
    connect(dropper, SIGNAL(picked(const QColor&)), SLOT(show()));
    connect(dropper, SIGNAL(canceled()), SLOT(show()));
    const qreal opacity = windowOpacity();
    setWindowOpacity(0.0); // Removes OS-specific animations on window hide
    hide();
    dropper->Exec();
    setWindowOpacity(opacity);
}

void ColorPickerDialog::OnOk()
{
    confirmed = true;
    emit changed(GetColor());
    close();
}

void ColorPickerDialog::UpdateControls(const QColor& c, AbstractColorPicker* source)
{
    for (auto it = pickers.begin(); it != pickers.end(); ++it)
    {
        AbstractColorPicker* recv = it.value();
        if (recv && recv != source)
        {
            recv->SetColor(c);
        }
    }
    for (auto it = colorSpaces.begin(); it != colorSpaces.end(); ++it)
    {
        AbstractColorPicker* recv = it.value();
        if (recv && recv != source)
        {
            recv->SetColor(c);
        }
    }

    ui->preview->SetColorNew(c);
    color = c;
}

void ColorPickerDialog::ConnectPicker(AbstractColorPicker* picker)
{
    connect(picker, SIGNAL(begin()), SIGNAL(begin()));
    connect(picker, SIGNAL(changing(const QColor&)), SLOT(OnChanging(const QColor&)));
    connect(picker, SIGNAL(changed(const QColor&)), SLOT(OnChanged(const QColor&)));
    connect(picker, SIGNAL(canceled()), SIGNAL(canceled()));
}

void ColorPickerDialog::closeEvent(QCloseEvent* e)
{
    if (modalLoop.isRunning())
    {
        modalLoop.quit();
    }

    QWidget::closeEvent(e);
}

void ColorPickerDialog::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        OnOk();
        break;
    }

    return QWidget::keyPressEvent(e);
}

void ColorPickerDialog::LoadSettings()
{
    DVASSERT(contextAccessor != nullptr);

    PropertiesItem propsItem = contextAccessor->CreatePropertiesNode(ColorPickerDialogDetail::PROPERTIES_KEY);

    {
        QRect loadedGeometry = propsItem.Get<QRect>(ColorPickerDialogDetail::GEOMETRY_KEY);
        if (loadedGeometry.isValid())
        {
            setGeometry(loadedGeometry);
            move(loadedGeometry.topLeft());
        }
    }

    {
        QByteArray paletteData = propsItem.Get<QByteArray>(ColorPickerDialogDetail::PALETTE_KEY);
        QDataStream paletteStream(&paletteData, QIODevice::ReadOnly);

        if (paletteData.size() != 0)
        { // load saved colors
            int32 n = paletteData.size() / sizeof(uint32);
            CustomPalette::Colors colors(n);
            for (int i = 0; i < n; i++)
            {
                uint32 c = 0;
                paletteStream >> c;
                colors[i] = QColor::fromRgba(c);
            }
            ui->customPalette->SetColors(colors);
        }
        else
        { // load default colors
            const DAVA::int32 n = Qt::darkYellow - Qt::black + 1;
            CustomPalette::Colors colors(n);
            for (int i = 0; i < n; i++)
            {
                colors[i] = QColor(Qt::GlobalColor(i + Qt::black));
            }
            ui->customPalette->SetColors(colors);
        }
    }

    DVASSERT(rgbam != nullptr);
    float32 maxMultiplier = propsItem.Get<float32>(ColorPickerDialogDetail::MULTIPLIER_KEY, 2.0f);
    rgbam->SetMaxMultiplierValue(maxMultiplier);
}

void ColorPickerDialog::SaveSettings()
{
    DVASSERT(contextAccessor != nullptr);

    PropertiesItem propsItem = contextAccessor->CreatePropertiesNode(ColorPickerDialogDetail::PROPERTIES_KEY);

    { //save palette
        QByteArray paletteData;
        QDataStream paletteStream(&paletteData, QIODevice::WriteOnly);

        const CustomPalette::Colors& colors = ui->customPalette->GetColors();
        for (int i = 0; i < colors.size(); i++)
        {
            paletteStream << colors[i].rgba();
        }

        propsItem.Set(ColorPickerDialogDetail::PALETTE_KEY, Any(paletteData));
    }

    {
        propsItem.Set(ColorPickerDialogDetail::GEOMETRY_KEY, Any(geometry()));
    }

    DVASSERT(rgbam != nullptr);
    propsItem.Set(ColorPickerDialogDetail::MULTIPLIER_KEY, static_cast<float32>(rgbam->GetMaxMultiplierValue()));
}
}
}
