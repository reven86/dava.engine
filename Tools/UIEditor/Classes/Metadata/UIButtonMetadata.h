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




#ifndef __UIEditor__ButtonNodeMetadata__
#define __UIEditor__ButtonNodeMetadata__

#include <QRectF>

#include "UITextControlMetadata.h"
#include "UI/UIButton.h"

namespace DAVA {
    
// Metadata class for DAVA UIButton control.
class UIButtonMetadata : public UITextControlMetadata
{
    Q_OBJECT

    Q_PROPERTY(bool Multiline READ GetMultiline WRITE SetMultiline);
    Q_PROPERTY(bool MultilineBySymbol READ GetMultilineBySymbol WRITE SetMultilineBySymbol);

public:
    UIButtonMetadata(QObject* parent = 0);    

protected:
    virtual bool GetInitialInputEnabled() const {return true;};

    // Initialization.
    virtual void InitializeControl(const String& controlName, const Vector2& position);
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);
    
    virtual QString GetUIControlClassName() const { return "UIButton"; };

    // Set the text of the button.
    virtual void SetLocalizedTextKey(const QString& value);
    virtual QString GetLocalizedTextKeyForState(UIControl::eControlState controlState) const;

    // Helper to access active UI Button.
    UIButton* GetActiveUIButton() const;
 
    virtual float GetFontSize() const;
    //virtual void SetFontSize(float fontSize);
    
    // Color getter/setter. Also virtual.
    virtual Font * GetFont() const;
    virtual void SetFont(Font* font);
    
    virtual QColor GetFontColor() const;
    virtual void SetFontColor(const QColor& value);
	
	// Shadow offset and color getters/setters
	virtual float GetShadowOffsetX() const;
	virtual void SetShadowOffsetX(float offset);
	
	virtual float GetShadowOffsetY() const;
	virtual void SetShadowOffsetY(float offset);
	
	virtual QColor GetShadowColor() const;
	virtual void SetShadowColor(const QColor& value);

    // Color getter/setter. Also virtual.
    virtual QColor GetColor() const;
    virtual void SetColor(const QColor& value);

    // Sprite getter/setter.
    virtual void SetSprite(const QString& value);
    virtual QString GetSprite() const;
    
    virtual void SetSpriteFrame(int value);
    virtual int GetSpriteFrame() const;
    
    // Drawing flags getters/setters.
    virtual int GetDrawType() const;
    virtual void SetDrawType(int value);

    virtual int GetColorInheritType() const;
    virtual void SetColorInheritType(int value);
    
    virtual int GetPerPixelAccuracyType() const;
    virtual void SetPerPixelAccuracyType(int value);

    virtual int GetAlign() const;
    virtual void SetAlign(int value);

	virtual void SetSpriteModification(int value);
	virtual int GetSpriteModification() const;

	virtual int GetTextAlign() const;
    virtual void SetTextAlign(int align);
	
	virtual bool GetTextUseRtlAlign();
    virtual void SetTextUseRtlAlign(bool value);

    virtual bool GetTextMultiline() const;
    virtual void SetTextMultiline(bool value);

    virtual bool GetTextMultilineBySymbol() const;
    virtual void SetTextMultilineBySymbol(bool value);

    virtual int GetFittingType() const;
    virtual void SetFittingType(int value);

    // Stretch Cap.
    virtual float GetLeftRightStretchCap() const;
	virtual void SetLeftRightStretchCap(float value);
    
    virtual float GetTopBottomStretchCap() const;
	virtual void SetTopBottomStretchCap(float value);

    // Color Inherit Type.
    virtual int GetTextColorInheritType() const;
    virtual void SetTextColorInheritType(int value);
	
    // Per pixel accuracy type
	virtual int GetTextPerPixelAccuracyType() const;
	virtual void SetTextPerPixelAccuracyType(int value);

    // Background Margins.
    virtual QRectF GetMargins() const;
    virtual void SetMargins(const QRectF& value);
    
    virtual float GetLeftMargin() const;
    virtual void SetLeftMargin(float value);
    
    virtual float GetTopMargin() const;
    virtual void SetTopMargin(float value);
    
    virtual float GetRightMargin() const;
    virtual void SetRightMargin(float value);
    
    virtual float GetBottomMargin() const;
    virtual void SetBottomMargin(float value);

    // Text Margins.
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

    // Multiline for button texts.
    virtual bool GetMultiline() const;
    virtual void SetMultiline(const bool value);
    
    virtual bool GetMultilineBySymbol() const;
    virtual void SetMultilineBySymbol(const bool value);

    // For UI Button localized text depends on state, so overriding this function.
    virtual UIControl::eControlState GetCurrentStateForLocalizedText() const;
    
    // Propery Dirty flags handling.
    // Localized Text.
    void UpdatePropertyDirtyFlagForLocalizedText();
    
    // Font property for selected state
    Font * GetFontForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForFont();

    // Font Size.
    float GetFontSizeForState(UIControl::eControlState state) const;
    //void UpdatePropertyDirtyFlagForFontSize();

    // Font Color.
    QColor GetFontColorForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForFontColor();

    // Color.
    QColor GetColorForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForColor();

    // Sprite Name.
    QString GetSpriteNameForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForSpriteName();
    
    // Sprite Frame.
    int GetSpriteFrameForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForSpriteFrame();

	// Text Align.
	int GetTextAlignForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForTextAlign();
	
	// Text use RTL align
	bool GetTextUseRtlAlignForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForTextUseRtlAlign();

    // Text multiline
    bool GetTextMultilineForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForTextMultiline();

    bool GetTextMultilineBySymbolForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForTextMultilineBySymbol();
	
    // Draw Type.
    void UpdatePropertyDirtyFlagForDrawType();
  
    // Color Inherit Type.
    int GetColorInheritTypeForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForColorInheritType();
    
    // Per pixel accuracy type
    int GetPerPixelAccuracyTypeForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForPerPixelAccuracyType();
    
    // Align Type.
    int GetAlignForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForAlign();

	// Sprite Modification Type
	int GetSpriteModificationForState(UIControl::eControlState state) const;
	void UpdatePropertyDirtyFlagForSpriteModification();

    // Fitting Type.
    int GetFittingTypeForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForFittingType();

    // Stretch Cap.
    float GetLeftRightStretchCapForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForLeftRightStretchCap();

    float GetTopBottomStretchCapForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForTopBottomStretchCap();

    // Margins.
    QRectF GetMarginsForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForMargins();

    // Text Margins.
    QRectF GetTextMarginsForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForTextMargins();

    virtual UIControlBackground::UIMargins GetMarginsToUpdate(UIControl::eControlState state) const;
    virtual UIControlBackground::UIMargins GetTextMarginsToUpdate(UIControl::eControlState state) const;

    // Shadow offset&color.
    Vector2 GetShadowOffsetXYForState(UIControl::eControlState state) const;
    QColor GetShadowColorForState(UIControl::eControlState state) const;

    // Font/shadow color inherit type.
    int GetTextColorInheritTypeForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForTextColorInheritType();
    
    // Font/shadow per pixel accuracy type.
    int GetTextPerPixelAccuracyTypeForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForTextPerPixelAccuracyType();
    
    void UpdatePropertyDirtyFlagForShadowOffsetXY();
    void UpdatePropertyDirtyFlagForShadowColor();

    // Multiline/multiline by symbol.
    bool GetMultilineForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForMultiline();

    bool GetMultilineBySymbolForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForMultilineBySymbol();

    // Recover dirty flags.
    void RecoverPropertyDirtyFlags();

    // Update the localization key.
    void UpdateExtraDataLocalizationKey();
};

};

#endif /* defined(__UIEditor__ButtonNodeMetadata__) */
