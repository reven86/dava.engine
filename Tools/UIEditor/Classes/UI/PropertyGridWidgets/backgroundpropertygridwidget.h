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


#ifndef BACKGROUNDEPROPERTYGRIDWIDGET_H
#define BACKGROUNDEPROPERTYGRIDWIDGET_H

#include "basepropertygridwidget.h"
#include <QWidget>

namespace Ui {
class BackgroundPropertyGridWidget;
}

class BackgroundPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit BackgroundPropertyGridWidget(const QString& controlName, const QString& propPrefix = QString(), QWidget *parent = 0);
    ~BackgroundPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

    void ForceExpand(bool value);
    
protected slots:
    void OnExpandButtonPressed();
    void OnOpenSpriteDialog();
    void OnRemoveSprite();

protected:
    void ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter, const QString& value);
    void UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty);

    void FillComboboxes();
    void UpdateDetailsWidget();
    
    void SetStretchCapMaxValues();
    void HandleDrawTypeComboBox();

    void CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value);

    // Get the property name with the prefix specified for this widget.
    String GetPrefixedPropertyName(const char* propertyName) const;

private:
    Ui::BackgroundPropertyGridWidget *ui;

    bool isDetailsVisible;
    String propertyPrefix;
};

#endif // BACKGROUNDEPROPERTYGRIDWIDGET_H
