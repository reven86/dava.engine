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



#ifndef UIEditor_UISliderMetadata_h
#define UIEditor_UISliderMetadata_h

#include "UIControlMetadata.h"
#include "UI/UISlider.h"

namespace DAVA {

// Metadata class for DAVA text-aware controls, responsible for the localization.
class UISliderMetadata : public UIControlMetadata
{
	Q_OBJECT

    // Slider Properties
    Q_PROPERTY(float SliderValue READ GetSliderValue WRITE SetSliderValue);
	Q_PROPERTY(float SliderMinValue READ GetSliderMinValue WRITE SetSliderMinValue);
	Q_PROPERTY(float SliderMaxValue READ GetSliderMaxValue WRITE SetSliderMaxValue);

    // Min Background properties.
    Q_PROPERTY(QColor MinBackgroundColor READ GetMinColor WRITE SetMinColor);
    Q_PROPERTY(QString MinSprite READ GetMinSprite WRITE SetMinSprite);
    Q_PROPERTY(int MinSpriteFrame READ GetMinSpriteFrame WRITE SetMinSpriteFrame);
	Q_PROPERTY(int MinSpriteModification READ GetMinSpriteModification WRITE SetMinSpriteModification);
    Q_PROPERTY(int MinDrawType READ GetMinDrawType WRITE SetMinDrawType);
    Q_PROPERTY(int MinColorInheritType READ GetMinColorInheritType WRITE SetMinColorInheritType);
    Q_PROPERTY(int MinPerPixelAccuracyType READ GetMinPerPixelAccuracyType WRITE SetMinPerPixelAccuracyType);
    Q_PROPERTY(int MinAlign READ GetMinAlign WRITE SetMinAlign);
    
	Q_PROPERTY(float MinLeftRightStretchCap READ GetMinLeftRightStretchCap WRITE SetMinLeftRightStretchCap);
	Q_PROPERTY(float MinTopBottomStretchCap READ GetMinTopBottomStretchCap WRITE SetMinTopBottomStretchCap);
    
    // Margins
    Q_PROPERTY(QRectF MinMargins READ GetMinMargins WRITE SetMinMargins);
    Q_PROPERTY(float MinLeftMargin READ GetMinLeftMargin WRITE SetMinLeftMargin);
	Q_PROPERTY(float MinTopMargin READ GetMinTopMargin WRITE SetMinTopMargin);
	Q_PROPERTY(float MinRightMargin READ GetMinRightMargin WRITE SetMinRightMargin);
	Q_PROPERTY(float MinBottomMargin READ GetMinBottomMargin WRITE SetMinBottomMargin);
    
    // Max Background Properties
    Q_PROPERTY(QColor MaxBackgroundColor READ GetMaxColor WRITE SetMaxColor);

    Q_PROPERTY(QString MaxSprite READ GetMaxSprite WRITE SetMaxSprite);
    Q_PROPERTY(int MaxSpriteFrame READ GetMaxSpriteFrame WRITE SetMaxSpriteFrame);
	Q_PROPERTY(int MaxSpriteModification READ GetMaxSpriteModification WRITE SetMaxSpriteModification);
    
    Q_PROPERTY(int MaxDrawType READ GetMaxDrawType WRITE SetMaxDrawType);
    Q_PROPERTY(int MaxColorInheritType READ GetMaxColorInheritType WRITE SetMaxColorInheritType);
    Q_PROPERTY(int MaxPerPixelAccuracyType READ GetMaxPerPixelAccuracyType WRITE SetMaxPerPixelAccuracyType);
    Q_PROPERTY(int MaxAlign READ GetMaxAlign WRITE SetMaxAlign);
    
	Q_PROPERTY(float MaxLeftRightStretchCap READ GetMaxLeftRightStretchCap WRITE SetMaxLeftRightStretchCap);
	Q_PROPERTY(float MaxTopBottomStretchCap READ GetMaxTopBottomStretchCap WRITE SetMaxTopBottomStretchCap);
    
    // Margins
    Q_PROPERTY(QRectF MaxMargins READ GetMaxMargins WRITE SetMaxMargins);
    Q_PROPERTY(float MaxLeftMargin READ GetMaxLeftMargin WRITE SetMaxLeftMargin);
	Q_PROPERTY(float MaxTopMargin READ GetMaxTopMargin WRITE SetMaxTopMargin);
	Q_PROPERTY(float MaxRightMargin READ GetMaxRightMargin WRITE SetMaxRightMargin);
	Q_PROPERTY(float MaxBottomMargin READ GetMaxBottomMargin WRITE SetMaxBottomMargin);

public:
    UISliderMetadata(QObject* parent = 0);
    
protected:
    virtual bool GetInitialInputEnabled() const {return true;};

    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);   
    virtual QString GetUIControlClassName() const { return "UISlider"; };
	// Override Resize function
	virtual void ApplyResize(const Rect& originalRect, const Rect& newRect);
	
	// Helper sanity check functions to avoid code duplication.
    bool VerifyActiveParamIDAndMinBackground() const;
    bool VerifyActiveParamIDAndMaxBackground() const;
    
    // Helper to access active UI Slider.
    UISlider* GetActiveUISlider() const;
	
    // Getters/setters.
    float GetSliderValue() const;
	void SetSliderValue(float value);
	
	// Max/Min value
    float GetSliderMinValue() const;
    void SetSliderMinValue(float value);
    float GetSliderMaxValue() const;
    void SetSliderMaxValue(float value);
    
    // Min Background.
    virtual QColor GetMinColor() const;
    virtual void SetMinColor(const QColor& value);
    
    virtual int GetMinDrawType() const;
    virtual void SetMinDrawType(int value);
    
    virtual int GetMinColorInheritType() const;
    virtual void SetMinColorInheritType(int value);
    
    virtual int GetMinPerPixelAccuracyType() const;
    virtual void SetMinPerPixelAccuracyType(int value);

    virtual int GetMinAlign() const;
    virtual void SetMinAlign(int value);
    
	virtual float GetMinLeftRightStretchCap() const;
	virtual void SetMinLeftRightStretchCap(float value);
	
	virtual float GetMinTopBottomStretchCap() const;
	virtual void SetMinTopBottomStretchCap(float value);

    virtual QString GetMinSprite() const;
    virtual void SetMinSprite(const QString& value);
    
    virtual void SetMinSpriteFrame(int value);
    virtual int GetMinSpriteFrame() const;

    virtual int GetMinSpriteModification() const;
	virtual void SetMinSpriteModification(int value);
    
    // Margins.
    QRectF GetMinMargins() const;
    void SetMinMargins(const QRectF& value);
    
    float GetMinLeftMargin() const;
    void SetMinLeftMargin(float value);
    
    float GetMinTopMargin() const;
    void SetMinTopMargin(float value);
    
    float GetMinRightMargin() const;
    void SetMinRightMargin(float value);
    
    float GetMinBottomMargin() const;
    void SetMinBottomMargin(float value);
    
    // Max Background.
    virtual QColor GetMaxColor() const;
    virtual void SetMaxColor(const QColor& value);

    virtual int GetMaxDrawType() const;
    virtual void SetMaxDrawType(int value);
    
    virtual int GetMaxColorInheritType() const;
    virtual void SetMaxColorInheritType(int value);
    
    virtual int GetMaxPerPixelAccuracyType() const;
    virtual void SetMaxPerPixelAccuracyType(int value);
    
    virtual int GetMaxAlign() const;
    virtual void SetMaxAlign(int value);
    
	virtual float GetMaxLeftRightStretchCap() const;
	virtual void SetMaxLeftRightStretchCap(float value);
	
	virtual float GetMaxTopBottomStretchCap() const;
	virtual void SetMaxTopBottomStretchCap(float value);
    
    virtual void SetMaxSprite(const QString& value);
    virtual QString GetMaxSprite() const;

    virtual int GetMaxSpriteFrame() const;
    virtual void SetMaxSpriteFrame(int value);

    virtual int GetMaxSpriteModification() const;
	virtual void SetMaxSpriteModification(int value);
    
    // Margins.
    QRectF GetMaxMargins() const;
    void SetMaxMargins(const QRectF& value);

    float GetMaxLeftMargin() const;
    void SetMaxLeftMargin(float value);

    float GetMaxTopMargin() const;
    void SetMaxTopMargin(float value);

    float GetMaxRightMargin() const;
    void SetMaxRightMargin(float value);

    float GetMaxBottomMargin() const;
    void SetMaxBottomMargin(float value);
    
    // Helper methods.
    QRectF GetMarginsForBackground(UIControlBackground* background) const;
    void SetMarginsForBackground(UIControlBackground* background, const QRectF& value);

    UIControlBackground::UIMargins GetMinMarginsToUpdate();
    UIControlBackground::UIMargins GetMaxMarginsToUpdate();
};

};

#endif
