#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QScopedPointer>
#include <QMap>
#include <QEventLoop>
#include <QPointer>

#include "Main/QtUtils.h"


namespace Ui
{
    class ColorPicker;
};

#include "AbstractColorPicker.h"


class EyeDropper;
class ColorPickerRGBAM;
class QtPosSaver;

class ColorPicker
    : public AbstractColorPicker
{
    Q_OBJECT

private:
    typedef QMap< QString, QPointer<AbstractColorPicker> > PickerMap;

public:
    explicit ColorPicker(QWidget* parent = 0);
    ~ColorPicker();

    bool Exec(const QString& title = QString());

    double GetMultiplierValue() const;
    void SetMultiplierValue(double val);

    void SetDavaColor(const DAVA::Color& color);
    DAVA::Color GetDavaColor() const;

    static double CalculateMultiplier( float r, float g, float b );
    static bool RemoveMultiplier( float& r, float& g, float& b );
    static void ApplyMultiplier( float& r, float& g, float& b, double mul );

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
    void keyPressEvent(QKeyEvent * e) override;

    void LoadCustomPalette();
    void SaveCustomPalette();

    QScopedPointer<Ui::ColorPicker> ui;
    QPointer<EyeDropper> dropper;
    QPointer<ColorPickerRGBAM> rgbam;
    PickerMap pickers;
    PickerMap colorSpaces;
    QEventLoop modalLoop;
    QColor oldColor;
    bool confirmed;
};

#endif // COLORPICKER_H
