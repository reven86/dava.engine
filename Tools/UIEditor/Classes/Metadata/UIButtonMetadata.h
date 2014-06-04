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

#include "UITextControlMetadata.h"
#include "UI/UIButton.h"

namespace DAVA {
    
// Metadata class for DAVA UIButton control.
class UIButtonMetadata : public UITextControlMetadata
{
    Q_OBJECT

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

    // Helper to access active UI Button.
    UIButton* GetActiveUIButton() const;
 
    virtual float GetFontSize() const;
    //virtual void SetFontSize(float fontSize);
    
    // Color getter/setter. Also virtual.
    virtual Font * GetFont();
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
    virtual QColor GetColor();
    virtual void SetColor(const QColor& value);

    // Sprite getter/setter.
    virtual void SetSprite(const QString& value);
    virtual QString GetSprite();
    
    virtual void SetSpriteFrame(int value);
    virtual int GetSpriteFrame();
    
    // Drawing flags getters/setters.
    virtual int GetDrawType();
    virtual void SetDrawType(int value);

    virtual int GetColorInheritType();
    virtual void SetColorInheritType(int value);

    virtual int GetAlign();
    virtual void SetAlign(int value);

	virtual void SetSpriteModification(int value);
	virtual int GetSpriteModification();

	virtual int GetTextAlign();
    virtual void SetTextAlign(int align);

    virtual int GetFittingType() const;
    virtual void SetFittingType(int value);

    // Stretch Cap.
    virtual float GetLeftRightStretchCap();
	virtual void SetLeftRightStretchCap(float value);
    
    virtual float GetTopBottomStretchCap();
	virtual void SetTopBottomStretchCap(float value);

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
    
    // Draw Type.
    void UpdatePropertyDirtyFlagForDrawType();
  
    // Color Inherit Type.
    int GetColorInheritTypeForState(UIControl::eControlState state) const;
    void UpdatePropertyDirtyFlagForColorInheritType();

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

    // Recover dirty flags.
    void RecoverPropertyDirtyFlags();
};

};

#endif /* defined(__UIEditor__ButtonNodeMetadata__) */
