#ifndef EDITPREVIEWSETTINGSDIALOG_H
#define EDITPREVIEWSETTINGSDIALOG_H

#include <QDialog>
#include "PreviewController.h"
using namespace DAVA;

namespace Ui {
class EditPreviewSettingsDialog;
}

class EditPreviewSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditPreviewSettingsDialog(QWidget *parent = 0);
    EditPreviewSettingsDialog();

    // Get the resulting preview settings data.
    const PreviewSettingsData& GetData() const;
    
protected slots:
    void OnOKButtonClicked();
    void OnCancelButtonClicked();

    void OnScreenHeightValueChanged(int value);
    void OnScreenWidthValueChanged(int value);

    void OnDPIValueChanged(int value);
    void OnDiagonalValueChanged(double value);

    // We can change either DPI or Diagonal value, but not both.
    void OnReferenceValueSetToDPI();
    void OnReferenceValueSetToDiagonal();

protected:
    // Do the validation of the input data.
    bool Validate();

    // Recalculate the screen params based on DPI/Diagonal/Screen Size.
    void RecalculateScreenParams();

    // Resulting preview settings data.
    PreviewSettingsData resultData;

private:
    Ui::EditPreviewSettingsDialog *ui;
    
    bool isReferenceValueDPI;
};

#endif // EDITPREVIEWSETTINGSDIALOG_H
