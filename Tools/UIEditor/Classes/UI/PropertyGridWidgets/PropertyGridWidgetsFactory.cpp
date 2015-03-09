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




#include "PropertyGridWidgetsFactory.h"

#include "PlatformMetadata.h"
#include "ScreenMetadata.h"
#include "AggregatorMetadata.h"

#include "UIControlMetadata.h"
#include "UIButtonMetadata.h"
#include "UIListMetadata.h"
#include "UIScrollViewMetadata.h"
#include "UIScrollBarMetadata.h"
#include "UISliderMetadata.h"
#include "UISpinnerMetadata.h"
#include "UIStaticTextMetadata.h"
#include "UISwitchMetadata.h"
#include "UITextFieldMetadata.h"
#include "UIParticlesMetadata.h"
#include "UIJoypadMetadata.h"
#include "UIWebViewMetadata.h"
#include "UI3DViewMetadata.h"
#include "UIListCellMetadata.h"

#include "Metadata/Custom/GuideMetadata.h"

#include "alignspropertygridwidget.h"
#include "backgroundpropertygridwidget.h"
#include "basepropertygridwidget.h"
#include "controlpropertygridwidget.h"
#include "flagspropertygridwidget.h"
#include "listpropertygridwidget.h"
#include "rectpropertygridwidget.h"
#include "scrollviewpropertygridwidget.h"
#include "scrollcontrolpropertygridwidget.h"
#include "sliderpropertygridwidget.h"
#include "spinnerpropertygridwidget.h"
#include "statepropertygridwidget.h"
#include "textpropertygridwidget.h"
#include "uitextfieldpropertygridwidget.h"
#include "uistatictextpropertygridwidget.h"
#include "particleeffectpropertygridwidget.h"
#include "joypadpropertygridwidget.h"
#include "webviewpropertygridwidget.h"
#include "UIListCellPropertyGridWidget.h"

#include "Custom/guidepropertygridwidget.h"

using namespace DAVA;

PropertyGridWidgetsFactory::PropertyGridWidgetsFactory()
{
    platformWidget = new PlatformPropertyGridWidget();
    registeredWidgets.push_back(platformWidget);
    
    screenWidget = new ScreenPropertyGridWidget();
    registeredWidgets.push_back(screenWidget);
	
	aggregatorWidget = new AggregatorPropertyGridWidget();
	registeredWidgets.push_back(aggregatorWidget);
    
    controlWidget = new ControlPropertyGridWidget();
    registeredWidgets.push_back(controlWidget);
    
    rectWidget = new RectPropertyGridWidget();
    registeredWidgets.push_back(rectWidget);
	
	alignWidget = new AlignsPropertyGridWidget();
	registeredWidgets.push_back(alignWidget);
  
    stateWidget = new StatePropertyGridWidget();
    registeredWidgets.push_back(stateWidget);
	
	sliderWidget = new SliderPropertyGridWidget();
	registeredWidgets.push_back(sliderWidget);

    textWidget = new UIStaticTextPropertyGridWidget();
    registeredWidgets.push_back(textWidget);

    uiTextFieldWidget = new UITextFieldPropertyGridWidget();
    registeredWidgets.push_back(uiTextFieldWidget);

    backgroundWidget = new BackgroundPropertyGridWidget("Background", "");
    backgroundWidget->ForceExpand(true);
    registeredWidgets.push_back(backgroundWidget);
	    
    flagsWidget = new FlagsPropertyGridWidget();
    registeredWidgets.push_back(flagsWidget);
	
	spinnerWidget = new SpinnerPropertyGridWidget();
	registeredWidgets.push_back(spinnerWidget);
	
	listWidget = new ListPropertyGridWidget();
	registeredWidgets.push_back(listWidget);
	
	scrollWidget = new ScrollControlPropertyGridWidget();
	registeredWidgets.push_back(scrollWidget);
	
	scrollViewWidget = new ScrollViewPropertyGridWidget();
	registeredWidgets.push_back(scrollViewWidget);
    
    particleWidget = new ParticleEffectPropertyGridWidget();
    registeredWidgets.push_back(particleWidget);

    joypadWidget = new JoypadPropertyGridWidget();
    registeredWidgets.push_back(joypadWidget);
    
    webViewWidget = new WebViewPropertyGridWidget();
    registeredWidgets.push_back(webViewWidget);

    listCellWidget = new UIListCellPropertyGridWidget();
    registeredWidgets.push_back(listCellWidget);

    guideWidget = new GuidePropertyGridWidget();
    registeredWidgets.push_back(guideWidget);
}

PropertyGridWidgetsFactory::~PropertyGridWidgetsFactory()
{
    for (PROPERTYGRIDWIDGETSITER iter = registeredWidgets.begin(); iter != registeredWidgets.end(); iter ++)
    {
        SAFE_DELETE(*iter);
    }
    
    registeredWidgets.clear();
}

void PropertyGridWidgetsFactory::InitializeWidgetParents(QWidget* parent)
{
    for (PROPERTYGRIDWIDGETSITER iter = registeredWidgets.begin(); iter != registeredWidgets.end(); iter ++)
    {
        (*iter)->setParent(parent);
        (*iter)->setVisible(false);
    }
}

const PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSLIST PropertyGridWidgetsFactory::GetWidgets(const BaseMetadata* metaData) const
{
    // Note - order here is important! The Property Widgets will be displayed exactly in the way
    // they are added to the list.
    PROPERTYGRIDWIDGETSLIST resultList;
    
    // Platform Node.
    const PlatformMetadata* platformMetadata = dynamic_cast<const PlatformMetadata*>(metaData);
    if (platformMetadata)
    {
        resultList.push_back(platformWidget);
        return resultList;
    }

    // Screen Node.
    const ScreenMetadata* screenMetadata = dynamic_cast<const ScreenMetadata*>(metaData);
    if (screenMetadata)
    {
        resultList.push_back(screenWidget);
        return resultList;
    }
	
	// aggregator node
	const AggregatorMetadata* aggregatorMetadata = dynamic_cast<const AggregatorMetadata*>(metaData);
	if (aggregatorMetadata)
	{
		resultList.push_back(aggregatorWidget);
		return resultList;
	}

    const UIListCellMetadata* listCellMetadata = dynamic_cast<const UIListCellMetadata*>(metaData);
    if (listCellMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(listCellWidget);
        resultList.push_back(rectWidget);
        resultList.push_back(alignWidget);
        resultList.push_back(stateWidget);
        resultList.push_back(textWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);

        return resultList;
    }

    // UI Button/Static Text Nodes - they require the same widgets.
    const UIButtonMetadata* uiButtonMetadata = dynamic_cast<const UIButtonMetadata*>(metaData);
    const UIStaticTextMetadata* uiStaticTextMetadata = dynamic_cast<const UIStaticTextMetadata*>(metaData);
    if (uiButtonMetadata || uiStaticTextMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
        resultList.push_back(alignWidget);
        if(uiButtonMetadata)
        {
            resultList.push_back(stateWidget);
        }
        resultList.push_back(textWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
    }
 
    // UI Text Field.
    const UITextFieldMetadata* uiTextFieldMetadata = dynamic_cast<const UITextFieldMetadata*>(metaData);
    if (uiTextFieldMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
        resultList.push_back(uiTextFieldWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
    }
	
	// Slider
	const UISliderMetadata* uiSliderMetadata = dynamic_cast<const UISliderMetadata*>(metaData);
	if (uiSliderMetadata)
	{
	    resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);		
		resultList.push_back(sliderWidget);
	 	resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
				
		return resultList;
	}

    // UI List.
	const UIListMetadata* uiListMetadata = dynamic_cast<const UIListMetadata*>(metaData);
    if (uiListMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
		resultList.push_back(listWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
    }

	// UI Spinner 
	const UISpinnerMetadata* uiSpinnerMetadata = dynamic_cast<const UISpinnerMetadata*>(metaData);
	
    if (uiSpinnerMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
    }
	
	// UI Switch
	const UISwitchMetadata* uiSwitchMetadata = dynamic_cast<const UISwitchMetadata*>(metaData);
	if (uiSwitchMetadata)
	{
		resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
	}
	
	// UI Scroll View.
	const UIScrollViewMetadata* uiScrollViewMetadata = dynamic_cast<const UIScrollViewMetadata*>(metaData);
	if (uiScrollViewMetadata)
	{
		resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
		resultList.push_back(scrollViewWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
	}
	
	// UI Scroll Bar
	const UIScrollBarMetadata* uiScrollBarMetadata = dynamic_cast<const UIScrollBarMetadata*>(metaData);
	if (uiScrollBarMetadata)
	{
		resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
		resultList.push_back(scrollWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
	}

    // UIParticles
	const UIParticlesMetadata* uiParticlesMetadata = dynamic_cast<const UIParticlesMetadata*>(metaData);
	if (uiParticlesMetadata)
	{
		resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
		resultList.push_back(particleWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
	}

    // UIJoypad
	const UIJoypadMetadata* uiJoypadMetadata = dynamic_cast<const UIJoypadMetadata*>(metaData);
	if (uiJoypadMetadata)
	{
		resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
		resultList.push_back(joypadWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);

        return resultList;
	}

    // UIWebView
	const UIWebViewMetadata* uiWebViewMetadata = dynamic_cast<const UIWebViewMetadata*>(metaData);
	if (uiWebViewMetadata)
	{
		resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
		resultList.push_back(webViewWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
	}

    // UI3DView - no background widget needed.
	const UI3DViewMetadata* ui3DViewMetadata = dynamic_cast<const UI3DViewMetadata*>(metaData);
	if (ui3DViewMetadata)
	{
		resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
	}

    // TODO: add other Metadatas here as soon as they will be implemented.
    // UI Control Node. Should be at the very bottom of this factory since it is a parent for
    // all UI Controls and used as a "last chance" if we are unable to determine the control type.
    const UIControlMetadata* uiControlMetadata = dynamic_cast<const UIControlMetadata*>(metaData);
    if (uiControlMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
    }

    // Handle Custom metadata here.
    const GuideMetadata* guideMetadata = dynamic_cast<const GuideMetadata*>(metaData);
    if (guideMetadata)
    {
        resultList.push_back(guideWidget);
        return resultList;
    }

    Logger::Error("No way to determine Property Grid Widgets for Metadata type %s!", typeid(metaData).name());
    return resultList;
}
