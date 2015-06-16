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


#ifndef __UIEditor__UIStaticTextControlMetadata__
#define __UIEditor__UIStaticTextControlMetadata__

#include "UIControlMetadata.h"
#include "UITextControlMetadata.h"

namespace DAVA {
    
// Metadata class for DAVA UIStaticText control.
class UIStaticTextMetadata : public UITextControlMetadata
{
    Q_OBJECT
	
public:
    UIStaticTextMetadata(QObject* parent = 0);

protected:
    virtual bool GetInitialInputEnabled() const {return false;}; // false because of DF-2944

    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);

    virtual QString GetUIControlClassName() const { return "UIStaticText"; };
    
    // Set the localized text key.
    virtual void SetLocalizedTextKey(const QString& value);

    // Access to the Static Text.
    UIStaticText* GetActiveStaticText() const;
    
    // Getters/setters.
	virtual int GetAlign() const;
    virtual void SetAlign(int value);

	virtual int GetTextAlign() const;
    virtual void SetTextAlign(int value);
	
	virtual bool GetTextUseRtlAlign();
	virtual void SetTextUseRtlAlign(bool value);

    virtual Font * GetFont() const;
    virtual void SetFont(Font* font);
    
    virtual float GetFontSize() const;
    //virtual void SetFontSize(float fontSize);
    
    virtual QColor GetFontColor() const;
    virtual void SetFontColor(const QColor& value);
	
	virtual float GetShadowOffsetX() const;
	virtual void SetShadowOffsetX(float offset);
	
	virtual float GetShadowOffsetY() const;
	virtual void SetShadowOffsetY(float offset);
	
	virtual QColor GetShadowColor() const;
	virtual void SetShadowColor(const QColor& value);

	virtual bool GetTextMultiline() const;
	virtual void SetTextMultiline(bool value);

	virtual bool GetTextMultilineBySymbol() const;
	virtual void SetTextMultilineBySymbol(bool value);
    
    virtual int GetFittingType() const;
    virtual void SetFittingType(int value);
    
    virtual int GetTextColorInheritType() const;
    virtual void SetTextColorInheritType(int value);

    virtual int GetTextPerPixelAccuracyType() const;
    virtual void SetTextPerPixelAccuracyType(int value);

    // Text margins.
    virtual QRectF GetTextMargins() const;
    virtual void SetTextMargins(const QRectF& value);
    
    virtual float GetTextLeftMargin() const;
    virtual void SetTextLeftMargin(float value);

    virtual float GetTextTopMargin() const;
    virtual void SetTextTopMargin(float value);
    
    virtual float GetTextRightMargin() const;
    virtual void SetTextRightMargin(float value);
    
    virtual float GetTextBottomMargin() const;
    virtual void SetTextBottomMargin(float value);

    virtual UIControlBackground::UIMargins GetTextMarginsToUpdate(UIControl::eControlState state = UIControl::STATE_NORMAL) const;
};

};

#endif /* defined(__UIEditor__UIStaticTextControlMetadata__) */
