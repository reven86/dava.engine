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



#include "BackgroundGridWidgetHelper.h"
using namespace DAVA;

const BackgroundGridWidgetHelper::DrawTypesData BackgroundGridWidgetHelper::drawTypesData[] =
{
    {UIControlBackground::DRAW_ALIGNED,                  "Aligned"},
    {UIControlBackground::DRAW_SCALE_TO_RECT,            "Scale to rect"},
    {UIControlBackground::DRAW_SCALE_PROPORTIONAL,       "Scale Proportional"},
    {UIControlBackground::DRAW_SCALE_PROPORTIONAL_ONE,   "Scale Proportional One"},
    {UIControlBackground::DRAW_FILL,                     "Fill"},
    {UIControlBackground::DRAW_STRETCH_HORIZONTAL,       "Stretch Horizontal"},
    {UIControlBackground::DRAW_STRETCH_VERTICAL,         "Stretch Vertical"},
    {UIControlBackground::DRAW_STRETCH_BOTH  ,           "Stretch Both"},
	{UIControlBackground::DRAW_TILED,					 "Tiled"}
};

const BackgroundGridWidgetHelper::ColorInheritTypesData BackgroundGridWidgetHelper::colorInheritTypesData[] =
{
    {UIControlBackground::COLOR_MULTIPLY_ON_PARENT,     "Multiply on Parent"},
    {UIControlBackground::COLOR_ADD_TO_PARENT,          "Add to Parent"},
    {UIControlBackground::COLOR_REPLACE_TO_PARENT,      "Replace to Parent"},
    {UIControlBackground::COLOR_IGNORE_PARENT,          "Ignore Parent"},
    {UIControlBackground::COLOR_MULTIPLY_ALPHA_ONLY,    "Multiply Alpha only"},
    {UIControlBackground::COLOR_REPLACE_ALPHA_ONLY,     "Replace Alpha only"}
};

const BackgroundGridWidgetHelper::PerPixelAccuracyTypesData BackgroundGridWidgetHelper::perPixelAccuracyTypesData[] =
{
    {UIControlBackground::PER_PIXEL_ACCURACY_DISABLED,	"Disabled"},
    {UIControlBackground::PER_PIXEL_ACCURACY_ENABLED,   "Enabled"},
    {UIControlBackground::PER_PIXEL_ACCURACY_FORCED,    "Forced"}
};

const BackgroundGridWidgetHelper::SpriteModificationTypesData BackgroundGridWidgetHelper::spriteModificationTypesData[] =
{
	{0,						"Original"},
	{ESM_HFLIP,				"Horizontal"},
	{ESM_VFLIP,				"Vertical"},
	{ESM_HFLIP|ESM_VFLIP,	"Both"}
};

const BackgroundGridWidgetHelper::AlignTypesData BackgroundGridWidgetHelper::alignTypesData[] =
{
    {ALIGN_LEFT | ALIGN_TOP,        "Left & Top"},
    {ALIGN_LEFT | ALIGN_VCENTER,    "Left & Vert Center"},
    {ALIGN_LEFT | ALIGN_BOTTOM,     "Left & Bottom"},

	{ALIGN_HCENTER | ALIGN_TOP,     "Horz Center & Top"},
    {ALIGN_HCENTER | ALIGN_VCENTER, "Horz Center & Vert Center"},
    {ALIGN_HCENTER | ALIGN_BOTTOM,  "Horz Center & Bottom"},

    {ALIGN_RIGHT | ALIGN_TOP,       "Right & Top"},
    {ALIGN_RIGHT | ALIGN_VCENTER,   "Right & Vert Center"},
    {ALIGN_RIGHT | ALIGN_BOTTOM,    "Right & Bottom"},
    
    {ALIGN_HJUSTIFY,                "Horizontal Justify"}
};

const BackgroundGridWidgetHelper::ReturnKeyTypesData BackgroundGridWidgetHelper::returnKeyTypesData[] =
{
	{UITextField::RETURN_KEY_DEFAULT,	"Default"},
	{UITextField::RETURN_KEY_GO,		"Go"},
	{UITextField::RETURN_KEY_GOOGLE,	"Google"},
	{UITextField::RETURN_KEY_JOIN,		"Join"},
	{UITextField::RETURN_KEY_NEXT,		"Next"},
	{UITextField::RETURN_KEY_ROUTE,		"Route"},
	{UITextField::RETURN_KEY_SEARCH,	"Search"},
	{UITextField::RETURN_KEY_SEND,		"Send"},
	{UITextField::RETURN_KEY_YAHOO,		"Yahoo"},
	{UITextField::RETURN_KEY_DONE,		"Done"},
	{UITextField::RETURN_KEY_EMERGENCY_CALL, "Emergency call"}
};

const BackgroundGridWidgetHelper::KeyboardTypesData BackgroundGridWidgetHelper::keyboardTypesData[] =
{
	{UITextField::KEYBOARD_TYPE_DEFAULT,		"Default"},
	{UITextField::KEYBOARD_TYPE_ASCII_CAPABLE,	"ASCII capatible"},
	{UITextField::KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION,	"Numbers & punctuation"},
	{UITextField::KEYBOARD_TYPE_URL,			"URL"},
	{UITextField::KEYBOARD_TYPE_NUMBER_PAD,		"Number pad"},
	{UITextField::KEYBOARD_TYPE_PHONE_PAD,		"Phone pad"},
	{UITextField::KEYBOARD_TYPE_NAME_PHONE_PAD,	"Name phone pad"},
	{UITextField::KEYBOARD_TYPE_EMAIL_ADDRESS,	"Email"},
	{UITextField::KEYBOARD_TYPE_DECIMAL_PAD,	"Decimal"},
	{UITextField::KEYBOARD_TYPE_TWITTER,		"Twitter"}
};

const BackgroundGridWidgetHelper::KeyboardAppearanceTypesData BackgroundGridWidgetHelper::keyboardAppearanceTypesData[] =
{
	{UITextField::KEYBOARD_APPEARANCE_DEFAULT,		"Default"},
	{UITextField::KEYBOARD_APPEARANCE_ALERT,		"Alert"}
};

const BackgroundGridWidgetHelper::SpellCheckingTypesData BackgroundGridWidgetHelper::spellCheckingTypesData[] =
{
	{UITextField::SPELL_CHECKING_TYPE_DEFAULT,	"Default"},
	{UITextField::SPELL_CHECKING_TYPE_NO,		"No"},
	{UITextField::SPELL_CHECKING_TYPE_YES,		"Yes"}
};

const BackgroundGridWidgetHelper::AutoCorrectionTypesData BackgroundGridWidgetHelper::autoCorrectionTypesData[] =
{
	{UITextField::AUTO_CORRECTION_TYPE_DEFAULT,		"Default"},
	{UITextField::AUTO_CORRECTION_TYPE_NO,		"No"},
	{UITextField::AUTO_CORRECTION_TYPE_YES,		"Yes"}
};

const BackgroundGridWidgetHelper::AutoCapitalizationTypesData BackgroundGridWidgetHelper::autoCapitalizationTypesData[] =
{
	{UITextField::AUTO_CAPITALIZATION_TYPE_NONE,		"None"},
	{UITextField::AUTO_CAPITALIZATION_TYPE_WORDS,		"Words"},
	{UITextField::AUTO_CAPITALIZATION_TYPE_SENTENCES,	"Sentences"},
	{UITextField::AUTO_CAPITALIZATION_TYPE_ALL_CHARS,	"All chars"}
};

const BackgroundGridWidgetHelper::FittingTypesData BackgroundGridWidgetHelper::fittingTypesData[] =
{
	{TextBlock::FITTING_DISABLED,   "Disabled"},
	{TextBlock::FITTING_ENLARGE,    "Enlarge"},
   	{TextBlock::FITTING_REDUCE,     "Reduce"},
   	{TextBlock::FITTING_POINTS,     "Add Points"},
    {TextBlock::FITTING_ENLARGE | TextBlock::FITTING_REDUCE,   "Enlarge & Reduce"}
};

int BackgroundGridWidgetHelper::GetDrawTypesCount()
{
    return sizeof(drawTypesData)/sizeof(*drawTypesData);
}

UIControlBackground::eDrawType BackgroundGridWidgetHelper::GetDrawType(int index)
{
    if (ValidateDrawTypeIndex(index) == false)
    {
        return  drawTypesData[0].drawType;
    }
    
    return drawTypesData[index].drawType;
}

QString BackgroundGridWidgetHelper::GetDrawTypeDesc(int index)
{
    if (ValidateDrawTypeIndex(index) == false)
    {
        return  drawTypesData[0].drawTypeDesc;
    }
    
    return drawTypesData[index].drawTypeDesc;
}

QString BackgroundGridWidgetHelper::GetDrawTypeDescByType(UIControlBackground::eDrawType drawType)
{
    int count = GetDrawTypesCount();
    for (int i = 0; i < count; i ++)
    {
        if (drawTypesData[i].drawType == drawType)
        {
            return drawTypesData[i].drawTypeDesc;
        }
    }
    
    Logger::Error("Unknown/unsupported Draw Type %i!", drawType);
    return QString();
}

int BackgroundGridWidgetHelper::GetColorInheritTypesCount()
{
    return sizeof(colorInheritTypesData)/sizeof(*colorInheritTypesData);
}

UIControlBackground::eColorInheritType BackgroundGridWidgetHelper::GetColorInheritType(int index)
{
    if (ValidateColorInheritTypeIndex(index) == false)
    {
        return  colorInheritTypesData[0].colorInheritType;
    }
    
    return colorInheritTypesData[index].colorInheritType;
}

QString BackgroundGridWidgetHelper::GetColorInheritTypeDesc(int index)
{
    if (ValidateColorInheritTypeIndex(index) == false)
    {
        return  colorInheritTypesData[0].colorInheritTypeDesc;
    }
    
    return colorInheritTypesData[index].colorInheritTypeDesc;
}

QString BackgroundGridWidgetHelper::GetColorInheritTypeDescByType(UIControlBackground::eColorInheritType inheritType)
{
    int count = GetColorInheritTypesCount();
    for (int i = 0; i < count; i ++)
    {
        if (colorInheritTypesData[i].colorInheritType == inheritType)
        {
            return colorInheritTypesData[i].colorInheritTypeDesc;
        }
    }
    
    Logger::Error("Unknown/unsupported Color Inherit Type %i!", inheritType);
    return QString();
}


int BackgroundGridWidgetHelper::GetPerPixelAccuracyTypesCount()
{
    return sizeof(perPixelAccuracyTypesData)/sizeof(*perPixelAccuracyTypesData);
}

UIControlBackground::ePerPixelAccuracyType BackgroundGridWidgetHelper::GetPerPixelAccuracyType(int index)
{
    if (ValidatePerPixelAccuracyTypeIndex(index) == false)
    {
        return  perPixelAccuracyTypesData[0].perPixelAccuracyType;
    }
    
    return perPixelAccuracyTypesData[index].perPixelAccuracyType;
}

QString BackgroundGridWidgetHelper::GetPerPixelAccuracyTypeDesc(int index)
{
    if (ValidatePerPixelAccuracyTypeIndex(index) == false)
    {
        return  perPixelAccuracyTypesData[0].perPixelAccuracyTypeDesc;
    }
    
    return perPixelAccuracyTypesData[index].perPixelAccuracyTypeDesc;
}

QString BackgroundGridWidgetHelper::GetPerPixelAccuracyTypeDescByType(UIControlBackground::ePerPixelAccuracyType pixelAccuracyType)
{
    int count = GetPerPixelAccuracyTypesCount();
    for (int i = 0; i < count; i ++)
    {
        if (perPixelAccuracyTypesData[i].perPixelAccuracyType == pixelAccuracyType)
        {
            return perPixelAccuracyTypesData[i].perPixelAccuracyTypeDesc;
        }
    }
    
    Logger::Error("Unknown/unsupported Pixel Accuracy Type %i!", pixelAccuracyType);
    return QString();
}

int BackgroundGridWidgetHelper::GetAlignTypesCount()
{
    return sizeof(alignTypesData)/sizeof(*alignTypesData);
}

int BackgroundGridWidgetHelper::GetAlignType(int index)
{
    if (ValidateAlginTypeIndex(index) == false)
    {
        return alignTypesData[0].alignType;
    }
    
    return alignTypesData[index].alignType;
}

QString BackgroundGridWidgetHelper::GetAlignTypeDesc(int index)
{
    if (ValidateAlginTypeIndex(index) == false)
    {
        return alignTypesData[0].alignTypeDesc;
    }
    
    return alignTypesData[index].alignTypeDesc;
}

// Get the Align Type Description by the Value.
QString BackgroundGridWidgetHelper::GetAlignTypeDescByType(int alignType)
{
    int count = GetAlignTypesCount();
    for (int i = 0; i < count; i ++)
    {
        if (alignTypesData[i].alignType == alignType)
        {
            return alignTypesData[i].alignTypeDesc;
        }
    }
    
    Logger::Error("Unknown/unsupported Align Type %i!", alignType);
    return QString();
}

//Return key type.
int BackgroundGridWidgetHelper::GetReturnKeyTypesCount()
{
    return sizeof(returnKeyTypesData)/sizeof(*returnKeyTypesData);
}

int BackgroundGridWidgetHelper::GetReturnKeyType(int index)
{
	if (ValidateReturnKeyTypeIndex(index) == false)
	{
		return returnKeyTypesData[0].returnKeyType;
	}
	
	return returnKeyTypesData[index].returnKeyType;
}

QString BackgroundGridWidgetHelper::GetReturnKeyTypeDesc(int index)
{
	if (ValidateReturnKeyTypeIndex(index) == false)
	{
		return returnKeyTypesData[0].returnKeyTypeDescription;
	}
	
	return returnKeyTypesData[index].returnKeyTypeDescription;
}

// Get the Return Key Type Description by the Value.
QString BackgroundGridWidgetHelper::GetReturnKeyTypeDescByType(int retKeyType)
{
	int count = GetReturnKeyTypesCount();
	for (int i = 0; i < count; i ++)
	{
		if (returnKeyTypesData[i].returnKeyType == retKeyType)
		{
			return returnKeyTypesData[i].returnKeyTypeDescription;
		}
	}
	
	Logger::Error("Unknown/unsupported Return key  Type %i!", retKeyType);
	return QString();
}


//Keyboard type.
int BackgroundGridWidgetHelper::GetKeyboardTypesCount()
{
    return sizeof(keyboardTypesData)/sizeof(*keyboardTypesData);
}

int BackgroundGridWidgetHelper::GetKeyboardType(int index)
{
	if (ValidateKeyboardTypeIndex(index) == false)
	{
		return keyboardTypesData[0].keyboardType;
	}
	
	return keyboardTypesData[index].keyboardType;
}

QString BackgroundGridWidgetHelper::GetKeyboardTypeDesc(int index)
{
	if (ValidateKeyboardTypeIndex(index) == false)
	{
		return keyboardTypesData[0].keyboardTypeDescription;
	}
	
	return keyboardTypesData[index].keyboardTypeDescription;
}

// Get the Keyboard Type Description by the Value.
QString BackgroundGridWidgetHelper::GetKeyboardTypeDescByType(int keyboardType)
{
	int count = GetKeyboardTypesCount();
	for (int i = 0; i < count; i ++)
	{
		if (keyboardTypesData[i].keyboardType == keyboardType)
		{
			return keyboardTypesData[i].keyboardTypeDescription;
		}
	}
	
	Logger::Error("Unknown/unsupported Keyboard Type %i!", keyboardType);
	return QString();
}

//Keyboard Appearance Type.
int BackgroundGridWidgetHelper::GetKeyboardAppearanceTypesCount()
{
	return sizeof(keyboardAppearanceTypesData)/sizeof(*keyboardAppearanceTypesData);
}

int BackgroundGridWidgetHelper::GetKeyboardAppearanceType(int index)
{
	if (ValidateKeyboardAppearanceIndex(index) == false)
	{
		return keyboardAppearanceTypesData[0].keyboardAppearanceType;
	}
	
	return keyboardAppearanceTypesData[index].keyboardAppearanceType;
}

QString BackgroundGridWidgetHelper::GetKeyboardAppearanceTypeDesc(int index)
{
	if (ValidateKeyboardAppearanceIndex(index) == false)
	{
		return keyboardAppearanceTypesData[0].keyboardAppearanceTypeDescription;
	}
	
	return keyboardAppearanceTypesData[index].keyboardAppearanceTypeDescription;
}
// Get the Keyboard Appearance Type Description by the Value.
QString BackgroundGridWidgetHelper::GetKeyboardAppearanceTypeDescByType(int keyboardAppearabveType)
{
	int count = GetKeyboardAppearanceTypesCount();
	for (int i = 0; i < count; i ++)
	{
		if (keyboardAppearanceTypesData[i].keyboardAppearanceType == keyboardAppearabveType)
		{
			return keyboardAppearanceTypesData[i].keyboardAppearanceTypeDescription;
		}
	}
	
	Logger::Error("Unknown/unsupported Keyboard Appearance Type %i!", keyboardAppearabveType);
	return QString();
}

//Spell Checking Type.
int BackgroundGridWidgetHelper::GetSpellCheckingTypesCount()
{
	return sizeof(spellCheckingTypesData)/sizeof(*spellCheckingTypesData);
}

int BackgroundGridWidgetHelper::GetSpellCheckingType(int index)
{
	if (ValidateSpellCheckingIndex(index) == false)
	{
		return spellCheckingTypesData[0].spellCheckingType;
	}
	
	return spellCheckingTypesData[index].spellCheckingType;
}
QString BackgroundGridWidgetHelper::GetSpellCheckingTypeDesc(int index)
{
	if (ValidateSpellCheckingIndex(index) == false)
	{
		return spellCheckingTypesData[0].spellCheckingTypeDescription;
	}
	
	return spellCheckingTypesData[index].spellCheckingTypeDescription;
}
// Get the SpellCheckingType Description by the Value.
QString BackgroundGridWidgetHelper::GetSpellCheckingTypeDescByType(int spellCheckingType)
{
	int count = GetSpellCheckingTypesCount();
	for (int i = 0; i < count; i ++)
	{
		if (spellCheckingTypesData[i].spellCheckingType == spellCheckingType)
		{
			return spellCheckingTypesData[i].spellCheckingTypeDescription;
		}
	}
	
	Logger::Error("Unknown/unsupported Spell checking Type %i!", spellCheckingType);
	return QString();
}

//	Auto Correction Type.
int BackgroundGridWidgetHelper::GetAutoCorrectionTypesCount()
{
	return sizeof(autoCorrectionTypesData)/sizeof(*autoCorrectionTypesData);
}

int BackgroundGridWidgetHelper::GetAutoCorrectionType(int index)
{
	if (ValidateAutoCorrectionIndex(index) == false)
	{
		return autoCorrectionTypesData[0].autoCorrectionType;
	}
	
	return autoCorrectionTypesData[index].autoCorrectionType;
}

QString BackgroundGridWidgetHelper::GetAutoCorrectionTypeDesc(int index)
{
	if (ValidateAutoCorrectionIndex(index) == false)
	{
		return autoCorrectionTypesData[0].autoCorrectionTypeDescription;
	}
	
	return autoCorrectionTypesData[index].autoCorrectionTypeDescription;
}

// Get the AutoCorrectionType Description by the Value.
QString BackgroundGridWidgetHelper::GetAutoCorrectionTypeDescByType(int autoCorrectionType)
{
	int count = GetAutoCorrectionTypesCount();
	for (int i = 0; i < count; i ++)
	{
		if (autoCorrectionTypesData[i].autoCorrectionType == autoCorrectionType)
		{
			return autoCorrectionTypesData[i].autoCorrectionTypeDescription;
		}
	}
	
	Logger::Error("Unknown/unsupported autoCorrection type %i!", autoCorrectionType);
	return QString();
}


//	Auto Capitalization Type.
int BackgroundGridWidgetHelper::GetAutoCapitalizationTypesCount()
{
	return sizeof(autoCapitalizationTypesData)/sizeof(*autoCapitalizationTypesData);
}

int BackgroundGridWidgetHelper::GetAutoCapitalizationType(int index)
{
	if (ValidateAutoCapitalizationIndex(index) == false)
	{
		return autoCapitalizationTypesData[0].autoCapitalizationType;
	}
	
	return autoCapitalizationTypesData[index].autoCapitalizationType;
}

QString BackgroundGridWidgetHelper::GetAutoCapitalizationTypeDesc(int index)
{
	if (ValidateAutoCapitalizationIndex(index) == false)
	{
		return autoCapitalizationTypesData[0].autoCapitalizationTypeDescription;
	}
	
	return autoCapitalizationTypesData[index].autoCapitalizationTypeDescription;
}
// Get the AutoCapitalizationType Description by the Value.
QString BackgroundGridWidgetHelper::GetAutoCapitalizationTypeDescByType(int autoCapitalizationType)
{
	int count = GetAutoCapitalizationTypesCount();
	for (int i = 0; i < count; i ++)
	{
		if (autoCapitalizationTypesData[i].autoCapitalizationType == autoCapitalizationType)
		{
			return autoCapitalizationTypesData[i].autoCapitalizationTypeDescription;
		}
	}
	
	Logger::Error("Unknown/unsupported autoCapitalization type %i!", autoCapitalizationType);
	return QString();
}

int BackgroundGridWidgetHelper::GetModificationType(int index)
{
	if (ValidateSpriteModificationIndex(index) == false)
    {
		return  spriteModificationTypesData[0].spriteModificationType;
    }
    
	return spriteModificationTypesData[index].spriteModificationType;
}

QString BackgroundGridWidgetHelper::GetModificationTypeDesc(int index)
{
	if (ValidateSpriteModificationIndex(index) == false)
    {
		return  spriteModificationTypesData[0].spriteModificationTypeDescription;
    }
    
    return spriteModificationTypesData[index].spriteModificationTypeDescription;
}

int BackgroundGridWidgetHelper::GetModificationTypesCount()
{
    return sizeof(spriteModificationTypesData)/sizeof(*spriteModificationTypesData);
}

QString BackgroundGridWidgetHelper::GetModificationTypeDescByType(int modifType)
{
	int count = GetModificationTypesCount();
    for (int i = 0; i < count; i ++)
    {
		if (spriteModificationTypesData[i].spriteModificationType == modifType)
        {
			return spriteModificationTypesData[i].spriteModificationTypeDescription;
        }
    }
    
    Logger::Error("Unknown/unsupported Modification Type %i!", modifType);
    return QString();
}

int BackgroundGridWidgetHelper::GetFittingTypesCount()
{
    return sizeof(fittingTypesData)/sizeof(*fittingTypesData);
}

int BackgroundGridWidgetHelper::GetFittingType(int index)
{
    if (!ValidateFittingTypeIndex(index))
    {
        return TextBlock::FITTING_DISABLED;
    }

    return fittingTypesData[index].fittingType;
}

QString BackgroundGridWidgetHelper::GetFittingTypeDesc(int index)
{
    if (!ValidateFittingTypeIndex(index))
    {
        return QString();
    }
    
    return fittingTypesData[index].fittingTypeDescription;
}

QString BackgroundGridWidgetHelper::GetFittingTypeDescByType(int fittingType)
{
	int count = GetFittingTypesCount();
    for (int i = 0; i < count; i ++)
    {
		if (fittingTypesData[i].fittingType == fittingType)
        {
			return fittingTypesData[i].fittingTypeDescription;
        }
    }
    
    Logger::Error("Unknown/unsupported Fitting Type %i!", fittingType);
    return QString();
}

bool BackgroundGridWidgetHelper::ValidateDrawTypeIndex(int index)
{
    if (index < 0 || index >= GetDrawTypesCount())
    {
        Logger::Error("Draw Type index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

bool BackgroundGridWidgetHelper::ValidateColorInheritTypeIndex(int index)
{
    if (index < 0 || index >= GetColorInheritTypesCount())
    {
        Logger::Error("Color Inherit Type index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

bool BackgroundGridWidgetHelper::ValidatePerPixelAccuracyTypeIndex(int index)
{
    if (index < 0 || index >= GetPerPixelAccuracyTypesCount())
    {
        Logger::Error("Per Pixel Accuracy Type index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

bool BackgroundGridWidgetHelper::ValidateAlginTypeIndex(int index)
{
    if (index < 0 || index >= GetAlignTypesCount())
    {
        Logger::Error("Align Type index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

bool BackgroundGridWidgetHelper::ValidateSpriteModificationIndex(int index)
{
	if (index < 0 || index >= GetModificationTypesCount())
    {
        Logger::Error("Sprite modification index %i is out of bounds!", index);
        return false;
    }
    
    return true;
}

bool BackgroundGridWidgetHelper::ValidateReturnKeyTypeIndex(int index)
{
	if (index < 0 || index >= GetReturnKeyTypesCount())
	{
		Logger::Error("Return keys index %i is out of bounds!", index);
		return false;
	}
	
	return true;
}

bool BackgroundGridWidgetHelper::ValidateKeyboardTypeIndex(int index)
{
	if (index < 0 || index >= GetKeyboardTypesCount())
	{
		Logger::Error("Keyboard types index %i is out of bounds!", index);
		return false;
	}
	
	return true;
}

bool BackgroundGridWidgetHelper::ValidateKeyboardAppearanceIndex(int index)
{
	if (index < 0 || index >= GetKeyboardAppearanceTypesCount())
	{
		Logger::Error("Keyboard Appearence index %i is out of bounds!", index);
		return false;
	}
	
	return true;
}

bool BackgroundGridWidgetHelper::ValidateSpellCheckingIndex(int index)
{
	if (index < 0 || index >= GetSpellCheckingTypesCount())
	{
		Logger::Error("Spell checking index %i is out of bounds!", index);
		return false;
	}
	
	return true;
}

bool BackgroundGridWidgetHelper::ValidateAutoCorrectionIndex(int index)
{
	if (index < 0 || index >= GetAutoCorrectionTypesCount())
	{
		Logger::Error("Autocorrection index %i is out of bounds!", index);
		return false;
	}
	
	return true;
}

bool BackgroundGridWidgetHelper::ValidateAutoCapitalizationIndex(int index)
{
	if (index < 0 || index >= GetAutoCapitalizationTypesCount())
	{
		Logger::Error("Autocapitalization index %i is out of bounds!", index);
		return false;
	}
	
	return true;
}

bool BackgroundGridWidgetHelper::ValidateFittingTypeIndex(int index)
{
	if (index < 0 || index >= GetFittingTypesCount())
	{
		Logger::Error("Fitting type index %i is out of bounds!", index);
		return false;
	}
	
	return true;
}

