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


#ifndef __WEB_VIEW_PROPERTY_GRID_WIDGET_H__
#define __WEB_VIEW_PROPERTY_GRID_WIDGET_H__

#include <QWidget>
#include "basepropertygridwidget.h"

namespace Ui {
    class WebViewPropertyGridWidget;
}

class WebViewPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT
    
public:
    explicit WebViewPropertyGridWidget(QWidget *parent = 0);
    ~WebViewPropertyGridWidget();
    
    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

protected:
    void FillControlsMap();
    void UpdateAvailableDataDetectorControls(bool isChecked, UIWebView::eDataDetectorType value);

    // Translate current data detector values to checkbox states and vice versa.
    int CheckboxesToDataDetectorTypes() const;
    void DataDetectorTypesToCheckboxes(int value);
    
    // Read/update the data detector types.
    void ReadDataDetectorTypes();
    void UpdateDataDetectorTypes(int value);

protected slots:
    void OnDataDetectorValueChanged(int value);
    
private:
    Map<QCheckBox*, UIWebView::eDataDetectorType> dataDetectorControlsMap;
    
    Ui::WebViewPropertyGridWidget *ui;
};

#endif // __WEB_VIEW_PROPERTY_GRID_WIDGET_H__
