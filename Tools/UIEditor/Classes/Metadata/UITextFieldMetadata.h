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


#ifndef UITEXTFIELDMETADATA_H
#define UITEXTFIELDMETADATA_H

#include "UITextControlMetadata.h"
#include "UI/UITextField.h"

namespace DAVA {

// Metadata class for DAVA text-field controls.
class UITextFieldMetadata : public UITextControlMetadata
{
    Q_OBJECT

    // Text field text Properties
    Q_PROPERTY(QString Text READ GetText WRITE SetText);
    Q_PROPERTY(QColor TextColor READ GetTextColor WRITE SetTextColor);
	Q_PROPERTY(bool IsPassword READ GetIsPassword WRITE SetIsPassword);
	
	Q_PROPERTY(int AutoCapitalizationType READ GetAutoCapitalizationType WRITE SetAutoCapitalizationType);
	Q_PROPERTY(int AutoCorrectionType READ GetAutoCorrectionType WRITE SetAutoCorrectionType);
	Q_PROPERTY(int SpellCheckingType READ GetSpellCheckingType WRITE SetSpellCheckingType);
	Q_PROPERTY(int KeyboardAppearanceType READ GetKeyboardAppearanceType WRITE SetKeyboardAppearanceType);
	Q_PROPERTY(int KeyboardType READ GetKeyboardType WRITE SetKeyboardType);
	Q_PROPERTY(int ReturnKeyType READ GetReturnKeyType WRITE SetReturnKeyType);
	Q_PROPERTY(bool IsReturnKeyAutomatically READ GetIsReturnKeyAutomatically WRITE SetIsReturnKeyAutomatically);
    
    Q_PROPERTY(int MaxLength READ GetMaxLength WRITE SetMaxLength);

public:
    UITextFieldMetadata(QObject* parent = 0);

protected:
    virtual bool GetInitialInputEnabled() const {return true;};
    
	virtual QString GetUIControlClassName() const { return "UITextField"; };
	
    // Initialization.
    void InitializeControl(const String& controlName, const Vector2& position);

    // Helper to access active UI Text Field.
    UITextField* GetActiveUITextField() const;
     // Getters/setters.
    QString GetText() const;
    void SetText(const QString& text);
    
    virtual Font * GetFont() const;
    virtual void SetFont(Font* font);

    virtual float GetFontSize() const;
    //virtual void SetFontSize(float fontSize);

    virtual QColor GetFontColor() const;
    virtual void SetFontColor(const QColor& value);

    QColor GetTextColor() const;
    void SetTextColor(const QColor& color);
	
	virtual float GetShadowOffsetX() const;
	virtual void SetShadowOffsetX(float offset);
	
	virtual float GetShadowOffsetY() const;
	virtual void SetShadowOffsetY(float offset);
	
	virtual QColor GetShadowColor() const;
	virtual void SetShadowColor(const QColor& value);

	virtual int GetTextAlign() const;
    virtual void SetTextAlign(int align);
	
	virtual bool GetTextUseRtlAlign();
	virtual void SetTextUseRtlAlign(bool value);

    virtual bool GetTextMultiline() const;
    virtual void SetTextMultiline(bool value);

    virtual bool GetTextMultilineBySymbol() const;
    virtual void SetTextMultilineBySymbol(bool value);
	
	bool GetIsPassword() const;
	void SetIsPassword(bool value);
	
	int GetAutoCapitalizationType() const;
	void SetAutoCapitalizationType(int value);
	
	int GetAutoCorrectionType() const;
	void SetAutoCorrectionType(int value);
	
	int GetSpellCheckingType() const;
	void SetSpellCheckingType(int value);
	
	int GetKeyboardAppearanceType() const;
	void SetKeyboardAppearanceType(int value);
	
	int GetKeyboardType() const;
	void SetKeyboardType(int value);
	
	int GetReturnKeyType() const;
	void SetReturnKeyType(int value);
	
	bool GetIsReturnKeyAutomatically() const;
	void SetIsReturnKeyAutomatically(bool value);

	int GetMaxLength() const;
	void SetMaxLength(int value);
};

};


#endif // UITEXTFIELDMETADATA_H
