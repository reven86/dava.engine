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
	// Text properties
    Q_PROPERTY(bool Multiline READ GetMultiline WRITE SetMultiline);
    Q_PROPERTY(bool MultilineBySymbol READ GetMultilineBySymbol WRITE SetMultilineBySymbol);

public:
    UIStaticTextMetadata(QObject* parent = 0);

protected:
    virtual bool GetInitialInputEnabled() const {return false;}; // false because of DF-2944

    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);

    virtual QString GetUIControlClassName() { return "UIStaticText"; };
    
    // Set the localized text key.
    virtual void SetLocalizedTextKey(const QString& value);

    // Access to the Static Text.
    UIStaticText* GetActiveStaticText() const;
    
    // Getters/setters.
	virtual int GetAlign();
    virtual void SetAlign(int value);

	virtual int GetTextAlign();
    virtual void SetTextAlign(int value);

    virtual Font * GetFont();
    virtual void SetFont(Font* font);
    
    virtual float GetFontSize() const;
    virtual void SetFontSize(float fontSize);
    
    virtual QColor GetFontColor() const;
    virtual void SetFontColor(const QColor& value);
	
	virtual float GetShadowOffsetX() const;
	virtual void SetShadowOffsetX(float offset);
	
	virtual float GetShadowOffsetY() const;
	virtual void SetShadowOffsetY(float offset);
	
	virtual QColor GetShadowColor() const;
	virtual void SetShadowColor(const QColor& value);

	virtual bool GetMultiline() const;
	virtual void SetMultiline(const bool value);

	virtual bool GetMultilineBySymbol() const;
	virtual void SetMultilineBySymbol(const bool value);
    
    virtual int GetFittingType() const;
    virtual void SetFittingType(int value);
};

};

#endif /* defined(__UIEditor__UIStaticTextControlMetadata__) */
