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





#ifndef __DAVAENGINE_UI_BUTTON_H__
#define __DAVAENGINE_UI_BUTTON_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"
#include "UI/UIStaticText.h"

namespace DAVA
{
    /**
     \ingroup controlsystem
     \brief Button.
        Use for different graphical representation of the control state.
        Every eControlState can be represented by the different UIControlBackground and different text.
        Only UIControlBackground for the STATE_NORMAL is present by default. No text for states is present by default.
        You should call one of the setter methods to create a UIControlBackground or a text control for a state.
        UIButton presents some accessors for the UITextControl and UIControlBackground, but for the full functionality
        you should use GetStateBackground() and GetStateTextControl().
     */

class UIButton : public UIControl
{

public:
    /**
     \brief Creates button with requested size and position.
     \param[in] rect Size and coordinates of control you want.
     \param[in] rectInAbsoluteCoordinates Send 'true' if you want to make control in screen coordinates.
        Rect coordinates will be recalculated to the hierarchy coordinates.
        Warning, rectInAbsoluteCoordinates isn't properly works for now!
     */
    UIButton(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);

    virtual void SetRect(const Rect &rect, bool rectInAbsoluteCoordinates = false );


    /**
     \brief Returns Sprite used for draw requested state in the UIControlBackground object.
     \param[in] state state to get value for.
     \returns Sprite used for draw.
     */
    virtual Sprite* GetStateSprite(int32 state);
    /**
     \brief Returns Sprite frame used for draw requested state in the UIControlBackground object.
     \param[in] state state to get value for.
     \returns Sprite frame used for draw.
     */
    virtual int32 GetStateFrame(int32 state);
    /**
     \brief Returns draw type used for draw requested state in the UIControlBackground object.
     \param[in] state state to get value for.
     \returns Draw type used for draw.
     */
    virtual UIControlBackground::eDrawType GetStateDrawType(int32 state);
    /**
     \brief Returns Sprite align used for draw requested state in the UIControlBackground object.
     \param[in] state state to get value for.
     \returns Sprite frame used for draw.
     */
    virtual int32 GetStateAlign(int32 state);
    /**
     \brief Sets Sprite for the control UIControlBackground object for the requested state.
        Method creates UIControlBackground object for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] spriteName Sprite path-name.
     \param[in] spriteFrame Sprite frame you want to use for draw.
     */
    virtual void SetStateSprite(int32 state, const FilePath &spriteName, int32 spriteFrame = 0);
    /**
     \brief Sets Sprite for the control UIControlBackground object for the requested state.
        Method creates UIControlBackground object for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] newSprite Pointer for a Sprite.
     \param[in] spriteFrame Sprite frame you want to use for draw.
     */
    virtual void SetStateSprite(int32 state, Sprite *newSprite, int32 spriteFrame = 0);
    /**
     \brief Sets Sprite frame you want to use for draw for the control UIControlBackground object for the requested state.
        Method creates UIControlBackground object for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] spriteFrame Sprite frame.
     */
    virtual void SetStateFrame(int32 state, int32 spriteFrame);
    /**
     \brief Sets draw type you want to use the control UIControlBackground object for the requested state.
        Method creates UIControlBackground object for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] drawType Draw type to use for drawing.
     */
    virtual void SetStateDrawType(int32 state, UIControlBackground::eDrawType drawType);
    /**
     \brief Sets Sprite align you want to use for draw for the control UIControlBackground object for the requested state.
        Method creates UIControlBackground object for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] align Sprite align.
     */
    virtual void SetStateAlign(int32 state, int32 align);
    /**
     \brief Sets Sprite reflection you want to use for draw for the control UIControlBackground object for the requested state.
        Method creates UIControlBackground object for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] modification Sprite modification(reflection).
     */
    virtual void SetStateModification(int32 state, int32 modification);
    /**
     \brief Sets Sprite color you want to use for draw for the control UIControlBackground object for the requested state.
        Method creates UIControlBackground object for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] color Sprite color.
     */
    virtual void SetStateColor(int32 state, Color color);
    /**
     \brief Sets Sprite's color inheritence type you want to use for draw for the control UIControlBackground object for the requested state.
        Method creates UIControlBackground object for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] value type of color inheritence.
     */
    virtual void SetStateColorInheritType(int32 state, UIControlBackground::eColorInheritType value);
    /**
     \brief Returns background used for drawing of the requested state.
     \param[in] state state to get value for.
     \returns background used for state draw.
     */
    virtual UIControlBackground *GetStateBackground(int32 state);
    /**
     \brief Sets background what will be used for draw of the requested states.
        Background is cloned inside button.
     \param[in] state state bit mask to set value for.
     \param[in] newBackground control background you want to use for draw of the states.
     */
    virtual void SetStateBackground(int32 state, UIControlBackground *newBackground);
    /**
     \brief Sets the fitting option for the text background of the requested states.
     \param[in] state state bit mask to set value for.
     \param[in] Fitting option.
     */
    virtual void SetStateFittingOption(int32 state, int32 fittingOption);
    /**
     \brief This method is invalid for the UIButton. Don't try to call this method.
     */
    virtual void SetBackground(UIControlBackground *newBg);
    /**
     \brief Returns UIControlBackground object actual for the current button state.
     \returns background used currently for draw.
     */
    virtual UIControlBackground * GetBackground();

    /**
     \brief Sets left align of control relative to its parent.
     \param[in] align left align of control.
     */
    virtual void SetLeftAlign(int32 align);
    /**
     \brief Sets horizontal central align of control relative to its parent.
     \param[in] align horizontal central align of control.
     */
    virtual void SetHCenterAlign(int32 align);
    /**
     \brief Sets right align of control relative to its parent.
     \param[in] align right align of control.
     */
    virtual void SetRightAlign(int32 align);
    /**
     \brief Sets top align of control relative to its parent.
     \param[in] align top align of control.
     */
    virtual void SetTopAlign(int32 align);
    /**
     \brief Sets vertical central align of control relative to its parent.
     \param[in] align l vertical central align of control.
     */
    virtual void SetVCenterAlign(int32 align);
    /**
     \brief Sets bottom align of control relative to its parent.
     \param[in] align bottom align of control.
     */
    virtual void SetBottomAlign(int32 align);

    /**
     \brief Sets background what will be used for draw of the requested states.
        Method creates UIStaticText control for the state if this is neccesary.
     \param[in] state state bit mask to set value for.
     \param[in] font font used for text draw of the states.
     */
    virtual void SetStateFont(int32 state, Font *font);

    /**
     \brief Sets the color of the font for particular state.
     \param[in] state state bit mask to set value for.
     \param[in] color font used for text draw of the states.
     */
    virtual void SetStateFontColor(int32 state, const Color& fontColor);

    /**
     \brief Sets the color of the shadow for particular state.
     \param[in] state state bit mask to set value for.
     \param[in] color font used for text draw of the states.
     */
    virtual void SetStateShadowColor(int32 state, const Color& shadowColor);

    /**
     \brief Sets the offset of the shadow for particular state.
     \param[in] state state bit mask to set value for.
     \param[in] color font used for text draw of the states.
     */
    virtual void SetStateShadowOffset(int32 state, const Vector2& offset);

    /**
     \brief Returns text control used for the requested state.
     \param[in] state state to get value for.
     \returns UIStaticText used for a state.
     */
    virtual UIStaticText * GetStateTextControl(int32 state);
    /**
     \brief Sets text what will be shown for the requested states.
     UIStaticText is cloned inside button.
     \param[in] state state bit mask to set value for.
     \param[in] text text you want to be shown for the text of the states.
     \param[in] requestedTextRectSize rect size to fit text in.
     */
    virtual void SetStateText(int32 state, const WideString &text, const Vector2 &requestedTextRectSize = Vector2(0,0));
    /**
     \brief Sets text align what will be shown for the requested states.
     \param[in] state state text bit mask to set value for.
     \param[in] align the align .
     */
    virtual void SetStateTextAlign(int32 state, int32 align);
    /**
     \brief Sets text control what will be used for the requested states.
        UIStaticText is cloned inside button.
     \param[in] state state bit mask to set value for.
     \param[in] textControl control you want to use for the text of the states.
     */
    virtual void SetStateTextControl(int32 state, UIStaticText *textControl);

    virtual void SetVisible(bool isVisible, bool hierarchic = true);
    virtual void SetVisibleForUIEditor(bool value, bool hierarchic = true);

    virtual void SetInputEnabled(bool isEnabled, bool hierarchic = true);
    virtual void SetDisabled(bool isDisabled, bool hierarchic = true);
    virtual void SetSelected(bool isSelected, bool hierarchic = true);
    virtual void SetExclusiveInput(bool isExclusiveInput, bool hierarchic = true);
    virtual void SetMultiInput(bool isMultiInput, bool hierarchic = true);

    virtual void Input(UIEvent *currentInput);

    virtual void SystemDraw(const UIGeometricData &geometricData);

    virtual UIControl *Clone();
    virtual void CopyDataFrom(UIControl *srcControl);
    /**
     \brief Creates the absoulutely identic copy of the button.
     \returns copy of the control
     */
    UIButton *CloneButton();


    virtual void SystemUpdate(float32 timeElapsed);// Internal method used by ControlSystem

    virtual void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader);
    virtual YamlNode * SaveToYamlNode(UIYamlLoader * loader);

    /**
     \brief Creates the background for the UIButton for particular state and caches it.
     Will create the background once only and then cache it.
     */
    virtual void CreateBackgroundForState(int32 state);

    /**
     \brief Returns list of control children without internal controls.
     \returns list of control children without internal controls.
     */
    virtual List<UIControl* >& GetRealChildren();

protected:
    enum eButtonDrawState
    {
            DRAW_STATE_UNPRESSED = 0
        ,	DRAW_STATE_PRESSED_INSIDE
        ,	DRAW_STATE_PRESSED_OUTSIDE
        ,	DRAW_STATE_DISABLED
        ,	DRAW_STATE_SELECTED
        ,	DRAW_STATE_HOVERED

        ,	DRAW_STATE_COUNT
    };
    UIControlBackground *stateBacks[DRAW_STATE_COUNT];
    UIStaticText    *stateTexts[DRAW_STATE_COUNT];

    int32 oldState;
    UIControlBackground *selectedBackground;
    UIStaticText	*selectedText;

    virtual ~UIButton();
    eButtonDrawState GetDrawStateForControlState(int32 state);
private:

    int32 BackgroundIndexForState(eButtonDrawState buttonState);
    UIControlBackground *GetActualBackground(int32 state);
    UIControlBackground *CreateBackForState(eButtonDrawState buttonState);

    virtual UIStaticText *CreateTextForState(eButtonDrawState buttonState);
    UIStaticText *GetActualText(int32 state);
    int32 TextIndexForState(eButtonDrawState buttonState);
    void UpdateStateTextControlSize();
};
};

#endif // __DAVAENGINE_BUTTON_H__




