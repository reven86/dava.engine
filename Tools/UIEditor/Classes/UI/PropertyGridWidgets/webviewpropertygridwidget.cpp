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


#include "webviewpropertygridwidget.h"
#include "ui_webviewpropertygridwidget.h"

#include "WidgetSignalsBlocker.h"
#include "PropertyNames.h"
#include "PropertiesHelper.h"

#include "ChangePropertyCommand.h"

static const QString WEBVIEW_PROPERTY_BLOCK_NAME = "Web View";

WebViewPropertyGridWidget::WebViewPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::WebViewPropertyGridWidget)
{
    ui->setupUi(this);

    SetPropertyBlockName(WEBVIEW_PROPERTY_BLOCK_NAME);
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

WebViewPropertyGridWidget::~WebViewPropertyGridWidget()
{
    delete ui;
}

void WebViewPropertyGridWidget::FillControlsMap()
{
    dataDetectorControlsMap[ui->allCheckbox] = UIWebView::DATA_DETECTOR_ALL;
    dataDetectorControlsMap[ui->noneCheckbox] = UIWebView::DATA_DETECTOR_NONE;
    dataDetectorControlsMap[ui->addressesCheckbox] = UIWebView::DATA_DETECTOR_ADDRESSES;
    dataDetectorControlsMap[ui->linksCheckbox] = UIWebView::DATA_DETECTOR_LINKS;
    dataDetectorControlsMap[ui->phoneNumbersCheckbox] =UIWebView::DATA_DETECTOR_PHONE_NUMBERS;
    dataDetectorControlsMap[ui->calendarEventsCheckbox] = UIWebView::DATA_DETECTOR_CALENDAR_EVENTS;
}

void WebViewPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    FillControlsMap();

    ReadDataDetectorTypes();

    for (Map<QCheckBox*, UIWebView::eDataDetectorType>::const_iterator iter = dataDetectorControlsMap.begin(); iter != dataDetectorControlsMap.end(); iter ++)
    {
        connect(iter->first, SIGNAL(stateChanged(int)), this, SLOT(OnDataDetectorValueChanged(int)));
    }
}

void WebViewPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    for (Map<QCheckBox*, UIWebView::eDataDetectorType>::const_iterator iter = dataDetectorControlsMap.begin();
         iter != dataDetectorControlsMap.end(); iter ++)
    {
        disconnect(iter->first, SIGNAL(stateChanged(int)), this, SLOT(OnDataDetectorValueChanged(int)));
    }

    dataDetectorControlsMap.clear();
}

void WebViewPropertyGridWidget::OnDataDetectorValueChanged(int /* value */)
{
    if (activeMetadata == NULL)
    {
        // No control already assinged.
        return;
    }
    
    QCheckBox* senderWidget = dynamic_cast<QCheckBox*>(QObject::sender());
    if (senderWidget == NULL)
    {
        return;
    }

    Map<QCheckBox*, UIWebView::eDataDetectorType>::const_iterator iter = dataDetectorControlsMap.find(senderWidget);
    DVASSERT(iter != dataDetectorControlsMap.end());
    
    bool isChecked = senderWidget->isChecked();
    UpdateAvailableDataDetectorControls(isChecked, iter->second);
    UpdateDataDetectorTypes(CheckboxesToDataDetectorTypes());
}

void WebViewPropertyGridWidget::UpdateAvailableDataDetectorControls(bool isChecked, UIWebView::eDataDetectorType value)
{
    if (!isChecked)
    {
        return;
    }

    bool isAllOrNoneSelected = (value == UIWebView::DATA_DETECTOR_ALL || value == UIWebView::DATA_DETECTOR_NONE);
    bool atLeastOneChecked = false;

    for (Map<QCheckBox*, UIWebView::eDataDetectorType>::const_iterator iter = dataDetectorControlsMap.begin();
         iter != dataDetectorControlsMap.end(); iter ++)
    {
        // Reset everything but All/None flags in case All/None is selected.
        // Reset both All and None flags in case anything else is selected.
        bool resetIfAllNoneSelected = isAllOrNoneSelected && (iter->second != value);
        bool resetIfAnythingElseSelected = !isAllOrNoneSelected &&
            ((iter->second == UIWebView::DATA_DETECTOR_NONE) || (iter->second == UIWebView::DATA_DETECTOR_ALL));

        if (resetIfAllNoneSelected || resetIfAnythingElseSelected)
        {
            WidgetSignalsBlocker blocker(iter->first);
            iter->first->setChecked(false);
        }

        if (iter->first->isChecked())
        {
            atLeastOneChecked = true;
        }
    }

    if (!atLeastOneChecked)
    {
        // Check the "None" flag in case nothing is checked.
        ui->noneCheckbox->setChecked(true);
    }
}

int WebViewPropertyGridWidget::CheckboxesToDataDetectorTypes() const
{
    int dataDetectorTypes = 0;
    for (Map<QCheckBox*, UIWebView::eDataDetectorType>::const_iterator iter = dataDetectorControlsMap.begin();
         iter != dataDetectorControlsMap.end(); iter ++)
    {
        if (iter->first->isEnabled() && iter->first->isChecked())
        {
            dataDetectorTypes |= iter->second;
        }
    }

    return dataDetectorTypes;
}

void WebViewPropertyGridWidget::DataDetectorTypesToCheckboxes(int value)
{
    for (Map<QCheckBox*, UIWebView::eDataDetectorType>::const_iterator iter = dataDetectorControlsMap.begin();
         iter != dataDetectorControlsMap.end(); iter ++)
    {
        WidgetSignalsBlocker blocker(iter->first);
        
        // UIWebView::DATA_DETECTOR_NONE has value 0 and can't be checked with a & operation.
        if (iter->second == UIWebView::DATA_DETECTOR_NONE)
        {
            iter->first->setChecked(value == iter->second);
        }
        else
        {
            iter->first->setChecked((value & iter->second) == iter->second);
        }
    }
    
    UpdateAvailableDataDetectorControls(ui->allCheckbox->isChecked(), dataDetectorControlsMap[ui->allCheckbox]);
    UpdateAvailableDataDetectorControls(ui->noneCheckbox->isChecked(), dataDetectorControlsMap[ui->noneCheckbox]);
}

void WebViewPropertyGridWidget::ReadDataDetectorTypes()
{
    int value = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata,
                                                            PropertyNames::WEBVIEW_DATA_DETECTOR_TYPES_PROPERTY_NAME);
    DataDetectorTypesToCheckboxes(value);
}

void WebViewPropertyGridWidget::UpdateDataDetectorTypes(int value)
{
	bool isPropertyValueDiffers = false;
    int curValue = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata,
                                                               PropertyNames::WEBVIEW_DATA_DETECTOR_TYPES_PROPERTY_NAME,
                                                               isPropertyValueDiffers);
	if ((curValue == value) && !isPropertyValueDiffers)
	{
		return;
	}

    int propertyIndex = activeMetadata->metaObject()->indexOfProperty(PropertyNames::WEBVIEW_DATA_DETECTOR_TYPES_PROPERTY_NAME);
    DVASSERT(propertyIndex != -1);

    PropertyGridWidgetData data(activeMetadata->metaObject()->property(propertyIndex), false);
    BaseCommand* command = new ChangePropertyCommand<int>(activeMetadata, data, value);
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
}
