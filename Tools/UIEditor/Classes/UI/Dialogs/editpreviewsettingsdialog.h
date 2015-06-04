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
    ~EditPreviewSettingsDialog();

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
