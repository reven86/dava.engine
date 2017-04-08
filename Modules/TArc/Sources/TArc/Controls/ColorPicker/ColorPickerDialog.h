#pragma once

#include "TArc/Controls/ColorPicker/Private/AbstractColorPicker.h"

#include <QWidget>
#include <QScopedPointer>
#include <QMap>
#include <QEventLoop>
#include <QPointer>

namespace Ui
{
class ColorPickerDialog;
};

namespace DAVA
{
class Color;

namespace TArc
{
class ContextAccessor;
class EyeDropper;
class ColorPickerRGBAM;

class ColorPickerDialog : public AbstractColorPicker
{
    Q_OBJECT

private:
    typedef QMap<QString, QPointer<AbstractColorPicker>> PickerMap;

public:
    explicit ColorPickerDialog(ContextAccessor* accessor, QWidget* parent = 0);
    ~ColorPickerDialog() override;

    bool Exec(const QString& title = QString());

    double GetMultiplierValue() const;
    void SetMultiplierValue(double val);

    void SetDavaColor(const Color& color);
    Color GetDavaColor() const;

    static double CalculateMultiplier(float r, float g, float b);
    static bool RemoveMultiplier(float& r, float& g, float& b);
    static void ApplyMultiplier(float& r, float& g, float& b, double mul);

protected:
    void RegisterPicker(const QString& key, AbstractColorPicker* picker);
    void RegisterColorSpace(const QString& key, AbstractColorPicker* picker);

    void SetColorInternal(const QColor& c) override;

private slots:
    void OnChanging(const QColor& c);
    void OnChanged(const QColor& c);
    void OnDropperChanged(const QColor& c);

    void OnDropper();
    void OnOk();

private:
    void UpdateControls(const QColor& c, AbstractColorPicker* source = nullptr);
    void ConnectPicker(AbstractColorPicker* picker);
    void closeEvent(QCloseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

    void LoadSettings();
    void SaveSettings();

    QScopedPointer<Ui::ColorPickerDialog> ui;
    QPointer<EyeDropper> dropper;
    QPointer<ColorPickerRGBAM> rgbam;
    PickerMap pickers;
    PickerMap colorSpaces;
    QEventLoop modalLoop;
    QColor oldColor;
    bool confirmed = false;

    ContextAccessor* contextAccessor = nullptr;
};
}
}
