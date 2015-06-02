/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "ColorPicker.h"
#include "ui_ColorPicker.h"

#include <QKeyEvent>
#include <QDebug>

#include "AbstractColorPicker.h"
#include "ColorPickerHSV.h"
#include "ColorPickerRGBAM.h"
#include "ColorPreview.h"
#include "EyeDropper.h"

#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Settings/SettingsManager.h"


ColorPicker::ColorPicker(QWidget* parent)
    : AbstractColorPicker(parent)
    , ui(new Ui::ColorPicker())
    , posSaver(new QtPosSaver())
    , confirmed(false)
{
    ui->setupUi(this);
    //posSaver->Attach(this); // Bugs with multiply monitors

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setFocusPolicy(Qt::ClickFocus);
    setFixedSize(size());

    // Pickers
    RegisterPicker("HSV rectangle", new ColorPickerHSV());

    // Editors
    rgbam = new ColorPickerRGBAM();
    RegisterColorSpace("RGBA M", rgbam);

    // Preview
    connect(this, SIGNAL( changing( const QColor& ) ), ui->preview, SLOT( SetColorNew( const QColor& ) ));
    connect(this, SIGNAL( changed( const QColor& ) ), ui->preview, SLOT( SetColorNew( const QColor& ) ));

    // Dropper
    connect(ui->dropper, SIGNAL( clicked() ), SLOT( OnDropper() ));

    // Color picker
    connect(ui->ok, SIGNAL( clicked() ), SLOT( OnOk() ));
    connect(ui->cancel, SIGNAL( clicked() ), SLOT( close() ));

    // Custom palette
    LoadCustomPalette();
    connect(ui->customPalette, SIGNAL( selected(const QColor&) ), SLOT( OnChanged(const QColor&) ));

    SetColor(Qt::white);
}

ColorPicker::~ColorPicker()
{
}

bool ColorPicker::Exec(const QString& title)
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

double ColorPicker::GetMultiplierValue() const
{
    if (rgbam)
    {
        return rgbam->GetMultiplierValue();
    }

    return 0.0;
}

void ColorPicker::SetMultiplierValue(double val)
{
    if (rgbam)
    {
        rgbam->SetMultiplierValue(val);
    }
}

void ColorPicker::SetDavaColor(const DAVA::Color& color)
{
    const QColor c = ColorToQColor(color);
    const double mul = CalculateMultiplier(color.r, color.g, color.b);

    SetColor(c);
    if (mul > 1.0)
    {
        SetMultiplierValue(mul);
    }
}

DAVA::Color ColorPicker::GetDavaColor() const
{
    const QColor c = GetColor();
    DAVA::Color newColor = QColorToColor(c);
    const double mul = GetMultiplierValue();
    ApplyMultiplier(newColor.r, newColor.g, newColor.b, mul);

    return newColor;
}

double ColorPicker::CalculateMultiplier(float r, float g, float b)
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

bool ColorPicker::RemoveMultiplier(float& r, float& g, float& b)
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

void ColorPicker::ApplyMultiplier(float& r, float& g, float& b, double mul)
{
    r *= mul;
    g *= mul;
    b *= mul;
}

void ColorPicker::RegisterPicker(QString const& key, AbstractColorPicker* picker)
{
    delete pickers[key];
    pickers[key] = picker;

    ui->pickerCombo->addItem(key, key);
    ui->pickerStack->addWidget(picker);
    ConnectPicker(picker);
}

void ColorPicker::RegisterColorSpace(const QString& key, AbstractColorPicker* picker)
{
    delete colorSpaces[key];
    colorSpaces[key] = picker;

    ui->colorSpaceCombo->addItem(key, key);
    ui->colorSpaceStack->addWidget(picker);
    ConnectPicker(picker);
}

void ColorPicker::SetColorInternal(const QColor& c)
{
    UpdateControls(c);
    oldColor = c;
    ui->preview->SetColorOld(c);
    ui->preview->SetColorNew(c);
}

void ColorPicker::OnChanging(const QColor& c)
{
    AbstractColorPicker* source = qobject_cast<AbstractColorPicker *>(sender());
    UpdateControls(c, source);
    emit changing(c);
}

void ColorPicker::OnChanged(const QColor& c)
{
    AbstractColorPicker* source = qobject_cast<AbstractColorPicker *>(sender());
    UpdateControls(c, source);
    emit changed(c);
}

void ColorPicker::OnDropperChanged(const QColor& c)
{
    QColor normalized(c);
    normalized.setAlphaF(GetColor().alphaF());
    UpdateControls(normalized);
    ui->preview->SetColorNew(normalized);
    emit changed(GetColor());
}

void ColorPicker::OnDropper()
{
    dropper = new EyeDropper(this);
    connect(dropper, SIGNAL( picked( const QColor& ) ), SLOT( OnDropperChanged( const QColor& ) ));
    connect(dropper, SIGNAL( picked( const QColor& ) ), SLOT( show() ));
    connect(dropper, SIGNAL( canceled() ), SLOT( show() ));
    const qreal opacity = windowOpacity();
    setWindowOpacity(0.0);      // Removes OS-specific animations on window hide
    hide();
    dropper->Exec();
    setWindowOpacity(opacity);
}

void ColorPicker::OnOk()
{
    confirmed = true;
    emit changed(GetColor());
    close();
}

void ColorPicker::UpdateControls(const QColor& c, AbstractColorPicker* source)
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

void ColorPicker::ConnectPicker(AbstractColorPicker* picker)
{
    connect(picker, SIGNAL( begin() ), SIGNAL( begin() ));
    connect(picker, SIGNAL( changing( const QColor& ) ), SLOT( OnChanging( const QColor& ) ));
    connect(picker, SIGNAL( changed( const QColor& ) ), SLOT( OnChanged( const QColor& ) ));
    connect(picker, SIGNAL( canceled() ), SIGNAL( canceled() ));
}

void ColorPicker::closeEvent(QCloseEvent* e)
{
    if (modalLoop.isRunning())
    {
        modalLoop.quit();
    }
    SaveCustomPalette();

    QWidget::closeEvent(e);
}

void ColorPicker::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
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

void ColorPicker::LoadCustomPalette()
{
    DAVA::VariantType v = SettingsManager::Instance()->GetValue(Settings::Internal_CustomPalette);
    const DAVA::int32 vSize = v.AsByteArraySize();
    const DAVA::int32 n = vSize / sizeof(DAVA::int32);
    const DAVA::uint32 *a = (DAVA::uint32 *)v.AsByteArray();

    CustomPalette::Colors colors(n);
    for (int i = 0; i < n; i++)
    {
        colors[i] = QColor::fromRgba(a[i]);
    }

    ui->customPalette->SetColors(colors);
}

void ColorPicker::SaveCustomPalette()
{
    const CustomPalette::Colors& colors = ui->customPalette->GetColors();
    const DAVA::int32 n = colors.size();
    QVector<DAVA::uint32> a(n);
    for (int i = 0; i < n; i++)
    {
        a[i] = colors[i].rgba();
    }

    const DAVA::VariantType v((DAVA::uint8 *)a.data(), n*sizeof(DAVA::uint32));
    SettingsManager::Instance()->SetValue(Settings::Internal_CustomPalette, v);
}
