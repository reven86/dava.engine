#ifndef __DIALOG_CONFIGURE_RESET_H__
#define __DIALOG_CONFIGURE_RESET_H__

#include "ui_DialogConfigurePreset.h"

namespace DAVA
{
class Font;
}

class DialogConfigurePreset : public QDialog, public Ui::DialogConfigurePreset
{
    Q_OBJECT

public:
    explicit DialogConfigurePreset(const QString& originalPresetName, QWidget* parent = nullptr);
    ~DialogConfigurePreset() = default;
private slots:
    void initPreset();
    void OnDefaultFontChanged(const QString& arg);
    void OnDefaultFontSizeChanged(int size);
    void OnLocalizedFontChanged(const QString& arg);
    void OnLocalizedFontSizeChanged(int size);

    void OnCurrentLocaleChanged(const QString& arg);
    void OnResetLocale();
    void OnApplyToAllLocales();
    void OnOk();
    void OnCancel();

private:
    void UpdateDefaultFontWidgets();
    void UpdateLocalizedFontWidgets();
    void SetFont(const QString& font, const int fontSize, const QString& locale);
    const QString originalPresetName;
};

#endif // __DIALOG_CONFIGURE_RESET_H__
