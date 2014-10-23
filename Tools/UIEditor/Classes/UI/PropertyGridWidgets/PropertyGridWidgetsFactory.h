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

#ifndef __UIEditor__PropertyGridWidgetsFactory__
#define __UIEditor__PropertyGridWidgetsFactory__

#include "Base/BaseTypes.h"

#include "BaseMetadata.h"

#include "platformpropertygridwidget.h"
#include "screenpropertygridwidget.h"
#include "aggregatorpropertygridwidget.h"

class ControlPropertyGridWidget;
class RectPropertyGridWidget;
class FlagsPropertyGridWidget;
class StatePropertyGridWidget;
class TextPropertyGridWidget;
class UITextFieldPropertyGridWidget;
class BackgroundPropertyGridWidget;
class SliderPropertyGridWidget;
class AlignsPropertyGridWidget;
class SpinnerPropertyGridWidget;
class ListPropertyGridWidget;
class ScrollControlPropertyGridWidget;
class ScrollViewPropertyGridWidget;
class ParticleEffectPropertyGridWidget;
class JoypadPropertyGridWidget;
class WebViewPropertyGridWidget;
class UIListCellPropertyGridWidget;
class GuidePropertyGridWidget;

namespace DAVA {

// Property Grid Widgets Factory - a class which stores pointers to all existing
// PropertyGridWidgets and returns the set needed for particular UI Control.
class PropertyGridWidgetsFactory
{
public:
    PropertyGridWidgetsFactory();
    ~PropertyGridWidgetsFactory();

    typedef List<BasePropertyGridWidget*> PROPERTYGRIDWIDGETSLIST;
    typedef PROPERTYGRIDWIDGETSLIST::iterator PROPERTYGRIDWIDGETSITER;
    
    // Initialize parents for all the widgets.
    void InitializeWidgetParents(QWidget* parent);
    
    // Get the ordered list of widgets to be displayed based on Metadata.
    const PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSLIST GetWidgets(const BaseMetadata* metaData) const;
 
private:
    // List of all PropertyGridWidgets.
    PlatformPropertyGridWidget* platformWidget;
    ScreenPropertyGridWidget* screenWidget;
	AggregatorPropertyGridWidget* aggregatorWidget;
    
    ControlPropertyGridWidget* controlWidget;
    RectPropertyGridWidget* rectWidget;
    FlagsPropertyGridWidget* flagsWidget;
    StatePropertyGridWidget* stateWidget;
    TextPropertyGridWidget* textWidget;
    UITextFieldPropertyGridWidget* uiTextFieldWidget;
    BackgroundPropertyGridWidget* backgroundWidget;
	SliderPropertyGridWidget* sliderWidget;
	AlignsPropertyGridWidget* alignWidget;
	SpinnerPropertyGridWidget* spinnerWidget;
	ListPropertyGridWidget* listWidget;
	ScrollControlPropertyGridWidget* scrollWidget;
	ScrollViewPropertyGridWidget* scrollViewWidget;
    ParticleEffectPropertyGridWidget* particleWidget;
    JoypadPropertyGridWidget* joypadWidget;
    WebViewPropertyGridWidget* webViewWidget;
    UIListCellPropertyGridWidget* listCellWidget;
    GuidePropertyGridWidget* guideWidget;

    // The same widgets in the list manner - for easier handling of group operations.
    PROPERTYGRIDWIDGETSLIST registeredWidgets;
};
    
}

#endif /* defined(__UIEditor__PropertyGridWidgetsFactory__) */
