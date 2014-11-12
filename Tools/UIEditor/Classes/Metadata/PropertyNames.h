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




#ifndef __UIEditor__PropertyNames__
#define __UIEditor__PropertyNames__

#include <QString>
namespace DAVA {
class PropertyNames
{
public:
    // Property Names for different Properties.
	static const char* SIZE_X;
	static const char* SIZE_Y;
	
    static const char* LOCALIZED_TEXT_KEY_PROPERTY_NAME;
    static const char* FONT_PROPERTY_NAME;
    static const char* FONT_SIZE_PROPERTY_NAME;
    static const char* FONT_COLOR_PROPERTY_NAME;
    
    static const char* BACKGROUND_COLOR_PROPERTY_NAME;
    static const char* SPRITE_PROPERTY_NAME;
    static const char* SPRITE_FRAME_PROPERTY_NAME;
	static const char* SPRITE_MODIFICATION_PROPERTY_NAME;

	static const char* STRETCH_HORIZONTAL_PROPERTY_NAME;
	static const char* STRETCH_VERTICAL_PROPERTY_NAME;
    
    static const char* DRAW_TYPE_PROPERTY_NAME;
    static const char* COLOR_INHERIT_TYPE_PROPERTY_NAME;
    static const char* PER_PIXEL_ACCURACY_TYPE_PROPERTY_NAME;
    static const char* ALIGN_PROPERTY_NAME;
    
    static const char* TEXT_PROPERTY_NAME;
	static const char* TEXT_PROPERTY_MULTILINE;
    static const char* TEXT_PROPERTY_MULTILINE_BY_SYMBOL;
    static const char* TEXT_COLOR_PROPERTY_NAME;
	static const char* TEXT_ALIGN_PROPERTY_NAME;
	static const char* TEXT_USE_RTL_ALIGN_PROPERTY_NAME;
	static const char* TEXT_FITTING_TYPE_PROPERTY_NAME;

	static const char* IS_PASSWORD_PROPERTY_NAME;
	
	static const char* AUTO_CAPITALIZATION_TYPE_PROPERTY_NAME;
	static const char* AUTO_CORRECTION_TYPE_PROPERTY_NAME;
	static const char* SPELL_CHECKING_TYPE_PROPERTY_NAME;
	static const char* KEYBOARD_APPEARANCE_TYPE_PROPERTY_NAME;
	static const char* KEYBOARD_TYPE_PROPERTY_NAME;
	static const char* RETURN_KEY_TYPE_PROPERTY_NAME;
	static const char* IS_RETURN_KEY_PROPERTY_NAME;

	static const char* SHADOW_OFFSET_X;
	static const char* SHADOW_OFFSET_Y;
	static const char* SHADOW_COLOR;
    
    static const char* TEXT_COLOR_INHERIT_TYPE_PROPERTY_NAME;
    static const char* TEXT_PER_PIXEL_ACCURACY_TYPE_PROPERTY_NAME;

    static const char* MAX_TEXT_LENGTH_PROPERTY_NAME;
    
	// Slider properties
	static const char* SLIDER_VALUE_PROPERTY_NAME;
	static const char* SLIDER_MIN_VALUE_PROPERTY_NAME;
	static const char* SLIDER_MAX_VALUE_PROPERTY_NAME;
	static const char* SLIDER_THUMB_SPRITE_PROPERTY_NAME;
	static const char* SLIDER_THUMB_SPRITE_FRAME_PROPERTY_NAME;
	static const char* SLIDER_MIN_SPRITE_PROPERTY_NAME;
	static const char* SLIDER_MIN_SPRITE_FRAME_PROPERTY_NAME;
	static const char* SLIDER_MIN_DRAW_TYPE_PROPERTY_NAME;
	static const char* SLIDER_MAX_SPRITE_PROPERTY_NAME;
	static const char* SLIDER_MAX_SPRITE_FRAME_PROPERTY_NAME;
	static const char* SLIDER_MAX_DRAW_TYPE_PROPERTY_NAME;
	
	// Align properties
	static const char* LEFT_ALIGN;
	static const char* LEFT_ALIGN_ENABLED;
	static const char* HCENTER_ALIGN;
	static const char* HCENTER_ALIGN_ENABLED;
	static const char* RIGHT_ALIGN;
	static const char* RIGHT_ALIGN_ENABLED;
	static const char* TOP_ALIGN;
	static const char* TOP_ALIGN_ENABLED;
	static const char* VCENTER_ALIGN;
	static const char* VCENTER_ALIGN_ENABLED;
	static const char* BOTTOM_ALIGN;
	static const char* BOTTOM_ALIGN_ENABLED;
	
	// UI Scroll View properties
	static const char* HORIZONTAL_SCROLL_POSITION;
	static const char* VERTICAL_SCROLL_POSITION;
	static const char* SCROLL_CONTENT_SIZE_X;
	static const char* SCROLL_CONTENT_SIZE_Y;
    
    //UI Scroll Bar properties
    static const char* SCROLL_ORIENTATION;
	static const char* SCROLL_BAR_DELEGATE_NAME;

	// UI Spinner properties.
	static const char* UISPINNER_PREV_BUTTON_TEXT;
	static const char* UISPINNER_NEXT_BUTTON_TEXT;

    // UI Partice effect properties and invokable methods.
    static const char* UIPARTICLES_AUTOSTART_PROPERTY;
    static const char* UIPARTICLES_EFFECT_PATH_PROPERTY;
    static const char* UIPARTICLES_START_DELAY_PROPERTY;

	static const char* UIPARTICLES_START_METHOD_NAME;
    static const char* UIPARTICLES_STOP_METHOD_NAME;
    static const char* UIPARTICLES_PAUSE_METHOD_NAME;
    static const char* UIPARTICLES_RESTART_METHOD_NAME;
    static const char* UIPARTICLES_RELOAD_METHOD_NAME;
    
    // UI Joypad properties.
    static const char* JOYPAD_STICK_SPRITE_PROPERTY_NAME;
    static const char* JOYPAD_STICK_SPRITE_FRAME_PROPERTY_NAME;
    static const char* JOYPAD_DEAD_AREA_PROPERTY_NAME;
    static const char* JOYPAD_DIGITAL_SENSE_PROPERTY_NAME;
    
    // UI Web View properties.
    static const char* WEBVIEW_DATA_DETECTOR_TYPES_PROPERTY_NAME;

    // UI Margins properties.
    static const char* MARGINS_PROPERTY_NAME;
    static const char* LEFT_MARGIN_PROPERTY_NAME;
    static const char* TOP_MARGIN_PROPERTY_NAME;
    static const char* RIGHT_MARGIN_PROPERTY_NAME;
    static const char* BOTTOM_MARGIN_PROPERTY_NAME;

    // UI Text Margins properties.
    static const char* TEXT_MARGINS_PROPERTY_NAME;
    static const char* TEXT_LEFT_MARGIN_PROPERTY_NAME;
    static const char* TEXT_TOP_MARGIN_PROPERTY_NAME;
    static const char* TEXT_RIGHT_MARGIN_PROPERTY_NAME;
    static const char* TEXT_BOTTOM_MARGIN_PROPERTY_NAME;

	// Custom Control properties.
	static const char* CUSTOM_CONTROL_NAME;
};

};

#endif /* defined(__UIEditor__PropertyNames__) */
