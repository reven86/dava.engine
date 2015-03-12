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


#ifndef __UIEditor__TextControlNodeMetadata__
#define __UIEditor__TextControlNodeMetadata__

#include "UIControlMetadata.h"
#include "UI/UIStaticText.h"

#include <QColor>

namespace DAVA {

// Metadata class for DAVA text-aware controls, responsible for the localization.
class UITextControlMetadata : public UIControlMetadata
{
    Q_OBJECT

    // Font Properties
    Q_PROPERTY(Font* Font READ GetFont WRITE SetFont);
    //Q_PROPERTY(float FontSize READ GetFontSize WRITE SetFontSize);
    Q_PROPERTY(float FontSize READ GetFontSize);
    Q_PROPERTY(QColor FontColor READ GetFontColor WRITE SetFontColor);

    Q_PROPERTY(QString LocalizedTextKey READ GetLocalizedTextKey WRITE SetLocalizedTextKey);
	
	Q_PROPERTY(float ShadowOffsetX READ GetShadowOffsetX WRITE SetShadowOffsetX);
	Q_PROPERTY(float ShadowOffsetY READ GetShadowOffsetY WRITE SetShadowOffsetY);
	Q_PROPERTY(QColor ShadowColor READ GetShadowColor WRITE SetShadowColor);
	Q_PROPERTY(int TextAlign READ GetTextAlign WRITE SetTextAlign);
	Q_PROPERTY(bool TextUseRtlAlign READ GetTextUseRtlAlign WRITE SetTextUseRtlAlign);
    Q_PROPERTY(int FittingType READ GetFittingType WRITE SetFittingType);

    // Font color/shadow color inherit types.
    Q_PROPERTY(int TextColorInheritType READ GetTextColorInheritType WRITE SetTextColorInheritType);
    // Font/Shadow per pixel accuracy types
    Q_PROPERTY(int TextPerPixelAccuracyType READ GetTextPerPixelAccuracyType WRITE SetTextPerPixelAccuracyType);

    // Text margins.
    Q_PROPERTY(QRectF TextMargins READ GetTextMargins WRITE SetTextMargins);
    Q_PROPERTY(float TextLeftMargin READ GetTextLeftMargin WRITE SetTextLeftMargin);
	Q_PROPERTY(float TextTopMargin READ GetTextTopMargin WRITE SetTextTopMargin);
	Q_PROPERTY(float TextRightMargin READ GetTextRightMargin WRITE SetTextRightMargin);
	Q_PROPERTY(float TextBottomMargin READ GetTextBottomMargin WRITE SetTextBottomMargin);

    // Text properties
    Q_PROPERTY(bool Multiline READ GetTextMultiline WRITE SetTextMultiline);
    Q_PROPERTY(bool MultilineBySymbol READ GetTextMultilineBySymbol WRITE SetTextMultilineBySymbol);

public:
    UITextControlMetadata(QObject* parent = 0);
    QString GetLocalizedTextValue();
    
protected:
    virtual QString GetLocalizedTextKey() const;
    virtual void SetLocalizedTextKey(const QString& value);
    
    // Getters/setters.
    virtual Font * GetFont() const = 0;
    virtual void SetFont(Font * font) = 0;

	virtual int GetTextAlign() const = 0;
    virtual void SetTextAlign(int align) = 0;

    virtual bool GetTextMultiline() const = 0;
    virtual void SetTextMultiline(bool value) = 0;

    virtual bool GetTextMultilineBySymbol() const = 0;
    virtual void SetTextMultilineBySymbol(bool value) = 0;
	
	virtual bool GetTextUseRtlAlign() = 0;
    virtual void SetTextUseRtlAlign(bool value) = 0;
    
    virtual float GetFontSize() const = 0;
    //virtual void SetFontSize(float fontSize) = 0;

    virtual QColor GetFontColor() const = 0;
    virtual void SetFontColor(const QColor& value) = 0;
	
	virtual float GetShadowOffsetX() const = 0;
	virtual void SetShadowOffsetX(float offset) = 0;
	
	virtual float GetShadowOffsetY() const = 0;
	virtual void SetShadowOffsetY(float offset) = 0;
	
	virtual QColor GetShadowColor() const = 0;
	virtual void SetShadowColor(const QColor& value) = 0;
	
	Vector2 GetOffsetX(const Vector2& currentOffset, float offsetX) const;
	Vector2 GetOffsetY(const Vector2& currentOffset, float offsetY) const;

    // Text margins. Should be overriden in the derived classes.
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

    // These methods should not be overriden for the UITextField, that's why not pure virtual.
    virtual int GetTextColorInheritType() const;
    virtual void SetTextColorInheritType(int value);
    
    virtual int GetTextPerPixelAccuracyType() const;
    virtual void SetTextPerPixelAccuracyType(int value);

    virtual int GetFittingType() const;
    virtual void SetFittingType(int value);

    // Get the localized text for particular control state.
    virtual QString GetLocalizedTextKeyForState(UIControl::eControlState controlState) const;
    
    // Update the static text extra data based on update style.
    void UpdateStaticTextExtraData(UIStaticText* staticText, UIControl::eControlState state,
                                   HierarchyTreeNodeExtraData& extraData,
                                   eExtraDataUpdateStyle updateStyle);
};

};

#endif /* defined(__UIEditor__TextControlNodeMetadata__)*/
