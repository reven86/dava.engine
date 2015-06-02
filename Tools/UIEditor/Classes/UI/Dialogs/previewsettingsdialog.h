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


#ifndef PREVIEWSETTINGSDIALOG_H
#define PREVIEWSETTINGSDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

#include "PreviewController.h"
using namespace DAVA;

namespace Ui {
class PreviewSettingsDialog;
}

class PreviewSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewSettingsDialog(bool selectionMode, QWidget *parent = 0);
    ~PreviewSettingsDialog();

    // Get the selected Preview Settings Data.
    PreviewSettingsData GetSelectedData() const;

    // Get the "apply scale" flag.
    bool GetApplyScale() const;

protected:
    void InitializeTableView();
    void ReloadSettings();
    
protected slots:
    void AddButtonClicked();
    void RemoveButtonClicked();
    void CloseButtonClicked();
    
    void PreviewNoScaleButtonClicked();
    void SettingsTableDoubleClicked(const QModelIndex& modelIndex);

private:
    Ui::PreviewSettingsDialog *ui;
    QStandardItemModel* tableModel;
    
    // true if this dialog is opened just for selection preview mode.
    bool isSelectionMode;
    
    // Whether apply scale is needed? TRUE by default.
    bool isApplyScale;
};

#endif // PREVIEWSETTINGSDIALOG_H
