#ifndef __DIALOG_ADD_PRESET_H__
#define __DIALOG_ADD_PRESET_H__

#include "ui_DialogAddPreset.h"

class DialogAddPreset : public QDialog, public Ui::DialogAddPreset
{
    Q_OBJECT
public:
    explicit DialogAddPreset(const QString& originalPresetName, QWidget* parent = nullptr);
    ~DialogAddPreset() = default;
private slots:
    void OnNewPresetNameChanged();
    void OnAccept();
};

#endif // __DIALOG_ADD_PRESET_H__
