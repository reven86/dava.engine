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


#ifndef __UIEditor__BackgroundGridWidgetHelper__
#define __UIEditor__BackgroundGridWidgetHelper__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UITextField.h"
#include <QString>

namespace DAVA {
    
class BackgroundGridWidgetHelper
{
public:
    // Draw Type.
    static int GetDrawTypesCount();
    static UIControlBackground::eDrawType GetDrawType(int index);
    static QString GetDrawTypeDesc(int index);

    // Get the Draw Type description by the Value.
    static QString GetDrawTypeDescByType(UIControlBackground::eDrawType drawType);

    // Color Inherit.
    static int GetColorInheritTypesCount();
    static UIControlBackground::eColorInheritType GetColorInheritType(int index);
    static QString GetColorInheritTypeDesc(int index);
    
    // Get the Color Inherit Type Description by the Value.
    static QString GetColorInheritTypeDescByType(UIControlBackground::eColorInheritType inheritType);
    
    // Per Pixer Accuracy.
    static int GetPerPixelAccuracyTypesCount();
    static UIControlBackground::ePerPixelAccuracyType GetPerPixelAccuracyType(int index);
    static QString GetPerPixelAccuracyTypeDesc(int index);
    
    // Get the Pixer Accuracy Type Description by the Value.
    static QString GetPerPixelAccuracyTypeDescByType(UIControlBackground::ePerPixelAccuracyType pixelAccuracyType);

    //Align.
    static int GetAlignTypesCount();
    static int GetAlignType(int index);
    static QString GetAlignTypeDesc(int index);

    // Get the Align Type Description by the Value.
    static QString GetAlignTypeDescByType(int alignType);

	//Return key type.
    static int GetReturnKeyTypesCount();
    static int GetReturnKeyType(int index);
    static QString GetReturnKeyTypeDesc(int index);
    // Get the Return Key Type Description by the Value.
    static QString GetReturnKeyTypeDescByType(int retKeyType);

	//Keyboard type.
    static int GetKeyboardTypesCount();
    static int GetKeyboardType(int index);
    static QString GetKeyboardTypeDesc(int index);
    // Get the Keyboard Type Description by the Value.
    static QString GetKeyboardTypeDescByType(int keyboardType);

	//Keyboard Appearance Type.
    static int GetKeyboardAppearanceTypesCount();
    static int GetKeyboardAppearanceType(int index);
    static QString GetKeyboardAppearanceTypeDesc(int index);
    // Get the Keyboard Appearance Type Description by the Value.
    static QString GetKeyboardAppearanceTypeDescByType(int keyboardAppearabveType);

	//Spell Checking Type.
    static int GetSpellCheckingTypesCount();
    static int GetSpellCheckingType(int index);
    static QString GetSpellCheckingTypeDesc(int index);
    // Get the SpellCheckingType Description by the Value.
    static QString GetSpellCheckingTypeDescByType(int spellCheckingType);

	//	Auto Correction Type.
    static int GetAutoCorrectionTypesCount();
    static int GetAutoCorrectionType(int index);
    static QString GetAutoCorrectionTypeDesc(int index);
    // Get the AutoCorrectionType Description by the Value.
    static QString GetAutoCorrectionTypeDescByType(int autoCorrectionType);

	
	//	Auto Capitalization Type.
    static int GetAutoCapitalizationTypesCount();
    static int GetAutoCapitalizationType(int index);
    static QString GetAutoCapitalizationTypeDesc(int index);
    // Get the AutoCapitalizationType Description by the Value.
    static QString GetAutoCapitalizationTypeDescByType(int autoCapitalizationType);

	//SpriteModificationType
	static int GetModificationType(int index);
    static QString GetModificationTypeDesc(int index);
	static int GetModificationTypesCount();

	static QString GetModificationTypeDescByType(int modificationType);

    // Fitting Type.
    static int GetFittingType(int index);
    static QString GetFittingTypeDesc(int index);
    static QString GetFittingTypeDescByType(int fittingType);
    static int GetFittingTypesCount();

protected:
    // Validate the indexes.
    static bool ValidateDrawTypeIndex(int index);
    static bool ValidateColorInheritTypeIndex(int index);
    static bool ValidatePerPixelAccuracyTypeIndex(int index);
    static bool ValidateAlginTypeIndex(int index);
	static bool ValidateSpriteModificationIndex(int index);

	static bool ValidateReturnKeyTypeIndex(int index);
	static bool ValidateKeyboardTypeIndex(int index);
	static bool ValidateKeyboardAppearanceIndex(int index);
	static bool ValidateSpellCheckingIndex(int index);
	static bool ValidateAutoCorrectionIndex(int index);
	static bool ValidateAutoCapitalizationIndex(int index);

    static bool ValidateFittingTypeIndex(int index);

    // Maps.
    struct DrawTypesData
    {
        UIControlBackground::eDrawType drawType;
        const char* drawTypeDesc;
    };
    
    struct ColorInheritTypesData
    {
        UIControlBackground::eColorInheritType colorInheritType;
        const char* colorInheritTypeDesc;
    };
    
    struct PerPixelAccuracyTypesData
    {
        UIControlBackground::ePerPixelAccuracyType perPixelAccuracyType;
        const char* perPixelAccuracyTypeDesc;
    };

    struct AlignTypesData
    {
        int alignType; // int, not eAlign, since align types combinations are possible.
        const char* alignTypeDesc;
    };

	struct SpriteModificationTypesData
    {
        int spriteModificationType;
        const char* spriteModificationTypeDescription;
    };
	
	struct ReturnKeyTypesData
	{
		UITextField::eReturnKeyType	returnKeyType;
		const char* returnKeyTypeDescription;
	};

	struct KeyboardTypesData
	{
		UITextField::eKeyboardType	keyboardType;
		const char* keyboardTypeDescription;
	};

	struct KeyboardAppearanceTypesData
	{
		UITextField::eKeyboardAppearanceType	keyboardAppearanceType;
		const char* keyboardAppearanceTypeDescription;
	};
		
	struct SpellCheckingTypesData
	{
		UITextField::eSpellCheckingType	spellCheckingType;
		const char* spellCheckingTypeDescription;
	};

	struct AutoCorrectionTypesData
	{
		UITextField::eAutoCorrectionType	autoCorrectionType;
		const char* autoCorrectionTypeDescription;
	};

	struct AutoCapitalizationTypesData
	{
		UITextField::eAutoCapitalizationType	autoCapitalizationType;
		const char* autoCapitalizationTypeDescription;
	};

    struct FittingTypesData
    {
        int fittingType; // int because combinations are possible.
        const char* fittingTypeDescription;
    };

    static const DrawTypesData drawTypesData[];
    static const ColorInheritTypesData colorInheritTypesData[];
    static const PerPixelAccuracyTypesData perPixelAccuracyTypesData[];
    static const AlignTypesData alignTypesData[];
	static const SpriteModificationTypesData spriteModificationTypesData[];
	
	static const ReturnKeyTypesData returnKeyTypesData[];
	static const KeyboardTypesData keyboardTypesData[];
	static const KeyboardAppearanceTypesData keyboardAppearanceTypesData[];
	static const SpellCheckingTypesData spellCheckingTypesData[];
	static const AutoCorrectionTypesData autoCorrectionTypesData[];
	static const AutoCapitalizationTypesData autoCapitalizationTypesData[];
    
    static const FittingTypesData fittingTypesData[];
};

};

#endif /* defined(__UIEditor__BackgroundGridWidgetHelper__) */
