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




#include "PropertyNames.h"
namespace DAVA {
// Property Names for different Properties.
const char* PropertyNames::SIZE_X = "SizeX";
const char* PropertyNames::SIZE_Y = "SizeY";
	
const char* PropertyNames::LOCALIZED_TEXT_KEY_PROPERTY_NAME = "LocalizedTextKey";
const char* PropertyNames::FONT_PROPERTY_NAME = "Font";
const char* PropertyNames::FONT_SIZE_PROPERTY_NAME = "FontSize";
const char* PropertyNames::FONT_COLOR_PROPERTY_NAME = "FontColor";

const char* PropertyNames::BACKGROUND_COLOR_PROPERTY_NAME = "BackgroundColor";
const char* PropertyNames::SPRITE_PROPERTY_NAME = "Sprite";
const char* PropertyNames::SPRITE_FRAME_PROPERTY_NAME = "SpriteFrame";
const char* PropertyNames::SPRITE_MODIFICATION_PROPERTY_NAME = "SpriteModification";

const char* PropertyNames::STRETCH_HORIZONTAL_PROPERTY_NAME = "LeftRightStretchCap";
const char* PropertyNames::STRETCH_VERTICAL_PROPERTY_NAME = "TopBottomStretchCap";

const char* PropertyNames::DRAW_TYPE_PROPERTY_NAME = "DrawType";
const char* PropertyNames::COLOR_INHERIT_TYPE_PROPERTY_NAME = "ColorInheritType";
const char* PropertyNames::ALIGN_PROPERTY_NAME = "Align";

const char* PropertyNames::TEXT_PROPERTY_NAME = "Text";
const char* PropertyNames::TEXT_PROPERTY_MULTILINE = "Multiline";
const char* PropertyNames::TEXT_PROPERTY_MULTILINE_BY_SYMBOL = "MultilineBySymbol";
const char* PropertyNames::TEXT_COLOR_PROPERTY_NAME = "TextColor";
const char* PropertyNames::TEXT_ALIGN_PROPERTY_NAME = "TextAlign";
const char* PropertyNames::TEXT_FITTING_TYPE_PROPERTY_NAME = "FittingType";

const char* PropertyNames::IS_PASSWORD_PROPERTY_NAME = "IsPassword";
	
const char* PropertyNames::AUTO_CAPITALIZATION_TYPE_PROPERTY_NAME = "AutoCapitalizationType";
const char* PropertyNames::AUTO_CORRECTION_TYPE_PROPERTY_NAME = "AutoCorrectionType";
const char* PropertyNames::SPELL_CHECKING_TYPE_PROPERTY_NAME = "SpellCheckingType";
const char* PropertyNames::KEYBOARD_APPEARANCE_TYPE_PROPERTY_NAME = "KeyboardAppearanceType";
const char* PropertyNames::KEYBOARD_TYPE_PROPERTY_NAME = "KeyboardType";
const char* PropertyNames::RETURN_KEY_TYPE_PROPERTY_NAME = "ReturnKeyType";
const char* PropertyNames::IS_RETURN_KEY_PROPERTY_NAME = "IsReturnKeyAutomatically";

const char* PropertyNames::SHADOW_OFFSET_X = "ShadowOffsetX";
const char* PropertyNames::SHADOW_OFFSET_Y = "ShadowOffsetY";
const char* PropertyNames::SHADOW_COLOR = "ShadowColor";

// Slider properties
const char* PropertyNames::SLIDER_VALUE_PROPERTY_NAME = "SliderValue";
const char* PropertyNames::SLIDER_MIN_VALUE_PROPERTY_NAME = "SliderMinValue";
const char* PropertyNames::SLIDER_MAX_VALUE_PROPERTY_NAME = "SliderMaxValue";
const char* PropertyNames::SLIDER_THUMB_SPRITE_PROPERTY_NAME = "SliderThumbSprite";
const char* PropertyNames::SLIDER_THUMB_SPRITE_FRAME_PROPERTY_NAME = "SliderThumbSpriteFrame";
const char* PropertyNames::SLIDER_MIN_SPRITE_PROPERTY_NAME = "SliderMinSprite";
const char* PropertyNames::SLIDER_MIN_SPRITE_FRAME_PROPERTY_NAME = "SliderMinSpriteFrame";
const char* PropertyNames::SLIDER_MIN_DRAW_TYPE_PROPERTY_NAME = "SliderMinDrawType";
const char* PropertyNames::SLIDER_MAX_SPRITE_PROPERTY_NAME = "SliderMaxSprite";
const char* PropertyNames::SLIDER_MAX_SPRITE_FRAME_PROPERTY_NAME = "SliderMaxSpriteFrame";
const char* PropertyNames::SLIDER_MAX_DRAW_TYPE_PROPERTY_NAME = "SliderMaxDrawType";

// Align properties
const char* PropertyNames::LEFT_ALIGN = "LeftAlign";
const char* PropertyNames::LEFT_ALIGN_ENABLED = "LeftAlignEnabled";
const char* PropertyNames::HCENTER_ALIGN = "HCenterAlign";
const char* PropertyNames::HCENTER_ALIGN_ENABLED = "HCenterAlignEnabled";
const char* PropertyNames::RIGHT_ALIGN = "RightAlign";
const char* PropertyNames::RIGHT_ALIGN_ENABLED = "RightAlignEnabled";
const char* PropertyNames::TOP_ALIGN = "TopAlign";
const char* PropertyNames::TOP_ALIGN_ENABLED = "TopAlignEnabled";
const char* PropertyNames::VCENTER_ALIGN = "VCenterAlign";
const char* PropertyNames::VCENTER_ALIGN_ENABLED = "VCenterAlignEnabled";
const char* PropertyNames::BOTTOM_ALIGN = "BottomAlign";
const char* PropertyNames::BOTTOM_ALIGN_ENABLED = "BottomAlignEnabled";

// UI Scroll View properties
const char* PropertyNames::HORIZONTAL_SCROLL_POSITION = "HorizontalScrollPosition";
const char* PropertyNames::VERTICAL_SCROLL_POSITION = "VerticalScrollPosition";
const char* PropertyNames::SCROLL_CONTENT_SIZE_X = "ContentSizeX";
const char* PropertyNames::SCROLL_CONTENT_SIZE_Y = "ContentSizeY";

// UI Spinner properties.
const char* PropertyNames::UISPINNER_PREV_BUTTON_TEXT = "PrevButtonText";
const char* PropertyNames::UISPINNER_NEXT_BUTTON_TEXT = "NextButtonText";

// UI Partice effect properties and invokable methods.
const char* PropertyNames::UIPARTICLES_AUTOSTART_PROPERTY = "Autostart";
const char* PropertyNames::UIPARTICLES_EFFECT_PATH_PROPERTY = "EffectPath";

const char* PropertyNames::UIPARTICLES_START_METHOD_NAME = "Start";
const char* PropertyNames::UIPARTICLES_STOP_METHOD_NAME = "Stop";
const char* PropertyNames::UIPARTICLES_RELOAD_METHOD_NAME = "Reload";

// Custom Control properties.
const char* PropertyNames::CUSTOM_CONTROL_NAME = "CustomControlName";

};
