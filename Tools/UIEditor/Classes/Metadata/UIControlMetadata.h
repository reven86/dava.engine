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



#ifndef __UIEditor__ControlNodeMetadata__
#define __UIEditor__ControlNodeMetadata__

#include "BaseMetadata.h"
#include "UI/UIControl.h"

#include <QColor>

namespace DAVA {

// Metadata common for all DAVA UI Controls.
class UIControlMetadata : public BaseMetadata
{
    Q_OBJECT
    
    // Properties which are common for all UIControls.
    Q_PROPERTY(QString UIControlClassName READ GetUIControlClassName);

    Q_PROPERTY(int Tag READ GetTag WRITE SetTag);
    
    // Size and position.
    Q_PROPERTY(float RelativeX READ GetRelativeX WRITE SetRelativeX);
    Q_PROPERTY(float RelativeY READ GetRelativeY WRITE SetRelativeY);

    Q_PROPERTY(float AbsoluteX READ GetAbsoluteX WRITE SetAbsoluteX);
    Q_PROPERTY(float AbsoluteY READ GetAbsoluteY WRITE SetAbsoluteY);

    Q_PROPERTY(float SizeX READ GetSizeX WRITE SetSizeX);
    Q_PROPERTY(float SizeY READ GetSizeY WRITE SetSizeY);

    Q_PROPERTY(float PivotX READ GetPivotX WRITE SetPivotX);
    Q_PROPERTY(float PivotY READ GetPivotY WRITE SetPivotY);
    Q_PROPERTY(QPointF Pivot READ GetPivot WRITE SetPivot);

    Q_PROPERTY(float Angle READ GetAngle WRITE SetAngle);

    // Background Properties.
    Q_PROPERTY(QColor BackgroundColor READ GetColor WRITE SetColor);
    
    Q_PROPERTY(QString Sprite READ GetSprite WRITE SetSprite);
    Q_PROPERTY(int SpriteFrame READ GetSpriteFrame WRITE SetSpriteFrame);
	Q_PROPERTY(int SpriteModification READ GetSpriteModification WRITE SetSpriteModification);
    
    Q_PROPERTY(int DrawType READ GetDrawType WRITE SetDrawType);
    Q_PROPERTY(int ColorInheritType READ GetColorInheritType WRITE SetColorInheritType);
    Q_PROPERTY(int Align READ GetAlign WRITE SetAlign);
    
	Q_PROPERTY(float LeftRightStretchCap READ GetLeftRightStretchCap WRITE SetLeftRightStretchCap);
	Q_PROPERTY(float TopBottomStretchCap READ GetTopBottomStretchCap WRITE SetTopBottomStretchCap);

    // Flag Properties
    Q_PROPERTY(bool Visible READ GetVisible WRITE SetVisible);
    Q_PROPERTY(bool Input READ GetInput WRITE SetInput);
    Q_PROPERTY(bool ClipContents READ GetClipContents WRITE SetClipContents);
	
	// Align Properties
	Q_PROPERTY(int LeftAlign READ GetLeftAlign WRITE SetLeftAlign);
	Q_PROPERTY(int HCenterAlign READ GetHCenterAlign WRITE SetHCenterAlign);
	Q_PROPERTY(int RightAlign READ GetRightAlign WRITE SetRightAlign);
	Q_PROPERTY(int TopAlign READ GetTopAlign WRITE SetTopAlign);
	Q_PROPERTY(int VCenterAlign READ GetVCenterAlign WRITE SetVCenterAlign);
	Q_PROPERTY(int BottomAlign READ GetBottomAlign WRITE SetBottomAlign);
	
	// Enable Align Properties
	Q_PROPERTY(bool LeftAlignEnabled READ GetLeftAlignEnabled WRITE SetLeftAlignEnabled);
	Q_PROPERTY(bool HCenterAlignEnabled READ GetHCenterAlignEnabled WRITE SetHCenterAlignEnabled);
	Q_PROPERTY(bool RightAlignEnabled READ GetRightAlignEnabled WRITE SetRightAlignEnabled);
	Q_PROPERTY(bool TopAlignEnabled READ GetTopAlignEnabled WRITE SetTopAlignEnabled);
	Q_PROPERTY(bool VCenterAlignEnabled READ GetVCenterAlignEnabled WRITE SetVCenterAlignEnabled);
	Q_PROPERTY(bool BottomAlignEnabled READ GetBottomAlignEnabled WRITE SetBottomAlignEnabled);

	// Initial State Property.
	Q_PROPERTY(int InitialState READ GetInitialState WRITE SetInitialState);

	// Custom Control Name property.
	Q_PROPERTY(QString CustomControlName READ GetCustomControlName WRITE SetCustomControlName);

public:
    UIControlMetadata(QObject* parent = 0);
    
    // Apply move/resize for all controls.
    virtual void ApplyMove(const Vector2& moveDelta);
    virtual void ApplyResize(const Rect& originalRect, const Rect& newRect);

protected:
    virtual QString GetUIControlClassName() { return "UIControl"; };
	
    // Default Flags.
    virtual bool GetInitialInputEnabled() const {return false;}; // false because of DF-2944

	virtual void InitializeControl(const String& controlName, const Vector2& position);

    // Getters/setters.
    virtual QString GetName() const;
    virtual void SetName(const QString& name);
    
    int GetTag() const;
    void SetTag(int tag);
    
    float GetRelativeX() const;
    void SetRelativeX(float value);
    float GetRelativeY() const;
    void SetRelativeY(float value);

    float GetAbsoluteX() const;
    void SetAbsoluteX(float value);
    float GetAbsoluteY() const;
    void SetAbsoluteY(float value);

    float GetSizeX() const;
    void SetSizeX(float value);
    float GetSizeY() const;
    void SetSizeY(float value);

    float GetPivotX() const;
    void SetPivotX(float value);
    float GetPivotY() const;
    void SetPivotY(float value);

    QPointF GetPivot() const;
    void SetPivot(const QPointF& value);

    float GetAngle() const;
    void SetAngle(float value);

    //Drawing flags getters/setters. Virtual because their implementation is different
    //for different control types.
    virtual int GetDrawType();
    virtual void SetDrawType(int value);
    
    virtual int GetColorInheritType();
    virtual void SetColorInheritType(int value);
    
    virtual int GetAlign();
    virtual void SetAlign(int value);

	virtual float GetLeftRightStretchCap();
	virtual void SetLeftRightStretchCap(float value);
	
	virtual float GetTopBottomStretchCap();
	virtual void SetTopBottomStretchCap(float value);

    //Color getter/setter. Also virtual.
    virtual QColor GetColor();
    virtual void SetColor(const QColor& value);

    // Sprite getter/setter. Also virtual one - its implementation is different
    // for different control types.
    virtual void SetSprite(const QString& value);
    virtual QString GetSprite();

    virtual void SetSpriteFrame(int value);
    virtual int GetSpriteFrame();

	virtual void SetSpriteModification(int value);
    virtual int GetSpriteModification();

    //Boolean gettes/setters
    bool GetVisible() const;
    virtual void SetVisible(const bool value);

    bool GetInput() const;
    void SetInput(const bool value);

    bool GetClipContents() const;
    void SetClipContents(const bool value);
	
	// Align getters/setters
	int GetLeftAlign();
	virtual void SetLeftAlign(int value);
	
	int GetHCenterAlign();
	virtual void SetHCenterAlign(int value);
	
	int GetRightAlign();
	virtual void SetRightAlign(int value);
	
	int GetTopAlign();
	virtual void SetTopAlign(int value);
	
	int GetVCenterAlign();
	virtual void SetVCenterAlign(int value);
	
	int GetBottomAlign();
	virtual void SetBottomAlign(int value);
	
	// Enable align getters/setters
	bool GetLeftAlignEnabled() const;
	virtual void SetLeftAlignEnabled(const bool value);
	
	bool GetHCenterAlignEnabled() const;
	virtual void SetHCenterAlignEnabled(const bool value);
	
	bool GetRightAlignEnabled() const;
	virtual void SetRightAlignEnabled(const bool value);
	
	bool GetTopAlignEnabled() const;
	virtual void SetTopAlignEnabled(const bool value);
	
	bool GetVCenterAlignEnabled() const;
	virtual void SetVCenterAlignEnabled(const bool value);
	
	bool GetBottomAlignEnabled() const;
	virtual void SetBottomAlignEnabled(const bool value);

	QString GetCustomControlName() const;
	void SetCustomControlName(const QString& value);

	// Initial state.
	int GetInitialState() const;
	void SetInitialState(int value);

	virtual void SetActiveControlRect(const Rect& rect, bool restoreAlign);

	// Refresh the align params.
	void RefreshAlign();

    // Refresh the thumb size for UISlider.
    void UpdateThumbSizeForUIControlThumb();

    // Verify whether UIControl exists and set its visible flag.
    void SetUIControlVisible(const bool isVisible, bool hierarchic);

private:
	void ResizeScrollViewContent(UIControl *control);

};
    
}

#endif /* defined(__UIEditor__ControlNodeMetadata__) */
