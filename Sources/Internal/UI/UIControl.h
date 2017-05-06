#pragma once

#include "Animation/AnimatedObject.h"
#include "Animation/Interpolation.h"
#include "Base/BaseTypes.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include "UI/UIControlBackground.h"
#include "UI/UIGeometricData.h"

namespace DAVA
{
class UIYamlLoader;
class Animation;
class EventDispatcher;
class UIEvent;
class UIControlBackground;
class Message;
class UIComponent;
class UIControlFamily;
class UIControlPackageContext;

#define CONTROL_TOUCH_AREA 15

/**
     \ingroup controlsystem
     \brief Base control system unit.
        Responsible for update, draw and user input processing.

        Methods call sequence:
        When the control adds to the hierarchy:

            -if hierarchy is allready on the screen SystemActive() will be called after adding control to hierarhy. SystemActive()
                calls OnActive() for the control and then calls SystemActive() for all control children.

            -if hierarchy is not on the screen methods would be called only when the hierarcy parent
                be placed to the screen.

        When the control removes from hierarchy:

            -SystemInactive() will be called. SystemInactive()
                calls OnInactive() for the control and then calls SystemInactive() for all control children.

        Every frame:

            -SystemDraw() is calls. SystemDraw() calculates current control geometric data. Transmit information
                about the parent color to the control background. Sets clip if requested. Calls Draw().
                Calls SystemDraw() for the all control children. Calls DrawAfterChilds(). Returns clip back.
                Draw() method proceed control background drawing by default.
                You can't remove, add or sort controls on the draw step.

        Every input:

            -SystemInput() is calls. At the first control process SystemInput() for all their children. If one
                of the children returns true from their SystemInput(), control is returns true too. If no one
                of the children returns true control is returns result of the SystemProcessInput() method.

            -SystemProcessInput() method checks if the control is responsible to process current input. If input
                is possible to process, SystemProcessInput() sets system flags and calls Input() method, then returns true.
                If input is inpossible to process SystemProcessInput() returns false.

        Each control contain UIControlBackground object responsible for visual
        representation. UIControlBackground can be changed for the custom. Or you can
        just overload Draw() method for the custom drawing.
     */
class UIControl : public AnimatedObject
{
    friend class UIInputSystem;
    friend class UIControlSystem;
    DAVA_VIRTUAL_REFLECTION(UIControl, AnimatedObject);

    // Need for isIteratorCorrupted. See UILayoutSystem::UpdateControl.
    friend class UILayoutSystem;

public:
    /**
     \enum Control state bits.
     */
    enum eControlState
    {
        STATE_NORMAL = 1 << 0, //!<Control isn't under influence of any activities.
        STATE_PRESSED_OUTSIDE = 1 << 1, //!<Mouse or touch comes into control but dragged outside of control.
        STATE_PRESSED_INSIDE = 1 << 2, //!<Mouse or touch comes into control.
        STATE_DISABLED = 1 << 3, //!<Control is disabled (don't process any input). Use this state only if you want change graphical representation of the control. Don't use this state for the disabling inputs for parts of the controls hierarchy!.
        STATE_SELECTED = 1 << 4, //!<Just a state for base control, nothing more.
        STATE_HOVER = 1 << 5, //!<This bit is rise then mouse is over the control.
        STATE_FOCUSED = 1 << 6, //!<Control under focus and will receive keyboard input. Additional this state can be used for setting visual style of control.

        STATE_COUNT = 7
    };

    static const char* STATE_NAMES[STATE_COUNT];

    /**
     \enum Control events supported by default.
     */
    enum eEventType
    {
        EVENT_TOUCH_DOWN = 1, //!<Trigger when mouse button or touch comes down inside the control.
        EVENT_TOUCH_UP_INSIDE = 2, //!<Trigger when mouse pressure or touch processed by the control is released.
        EVENT_VALUE_CHANGED = 3, //!<Used with sliders, spinners and switches. Trigger when value of the control is changed. Non-NULL callerData means that value is changed from code, not from UI.
        EVENT_HOVERED_SET = 4, //!<
        EVENT_HOVERED_REMOVED = 5, //!<
        EVENT_FOCUS_SET = 6, //!<Trigger when control becomes focused
        EVENT_FOCUS_LOST = 7, //!<Trigger when control losts focus
        EVENT_TOUCH_UP_OUTSIDE = 8, //!<Trigger when mouse pressure or touch processed by the control is released outside of the control.
        EVENTS_COUNT
    };

    enum eDebugDrawPivotMode
    {
        DRAW_NEVER = 1, //!<Never draw the Pivot Point.
        DRAW_ONLY_IF_NONZERO, //!<Draw the Pivot Point only if it is defined (nonzero).
        DRAW_ALWAYS //!<Always draw the Pivot Point mark.
    };

public:
    /**
     \brief Creates control with requested size and position.
     \param[in] rect Size and coordinates of control you want.
     \param[in] rectInAbsoluteCoordinates Send 'true' if you want to make control in screen coordinates.
        Rect coordinates will be recalculated to the hierarchy coordinates.
        Warning, rectInAbsoluteCoordinates isn't properly works for now!
     */
    UIControl(const Rect& rect = Rect());

    /**
     \brief Returns Sprite used for draw in the current UIControlBackground object.
        You can call this function directly for the controlBackgound.
     \returns Sprite used for draw.
     */
    DAVA_DEPRECATED(Sprite* GetSprite() const);
    /**
     \brief Returns Sprite frame used for draw in the current UIControlBackground object.
        You can call this function directly for the controlBackgound.
     \returns Sprite frame used for draw.
     */
    DAVA_DEPRECATED(int32 GetFrame() const);
    /**
     \brief Returns draw type used for draw in the current UIControlBackground object.
        You can call this function directly for the controlBackgound.
     \returns Draw type used for draw.
     */
    DAVA_DEPRECATED(virtual UIControlBackground::eDrawType GetSpriteDrawType() const);
    /**
     \brief Returns Sprite align used for draw in the current UIControlBackground object.
        You can call this function directly for the controlBackgound.
     \returns Sprite eAlign bit mask used for draw.
     */
    DAVA_DEPRECATED(virtual int32 GetSpriteAlign() const);
    /**
     \brief Sets Sprite for the control UIControlBackground object.
     \param[in] spriteName Sprite path-name.
     \param[in] spriteFrame Sprite frame you want to use for draw.
     */
    DAVA_DEPRECATED(virtual void SetSprite(const FilePath& spriteName, int32 spriteFrame));
    /**
     \brief Sets Sprite for the control UIControlBackground object.
     \param[in] newSprite Pointer for a Sprite.
     \param[in] spriteFrame Sprite frame you want to use for draw.
     */
    DAVA_DEPRECATED(virtual void SetSprite(Sprite* newSprite, int32 spriteFrame));
    /**
     \brief Sets Sprite frame you want to use for draw for the control UIControlBackground object.
     \param[in] spriteFrame Sprite frame.
     */
    DAVA_DEPRECATED(virtual void SetSpriteFrame(int32 spriteFrame));
    /**
     \brief Sets Sprite frame you want to use for draw for the control UIControlBackground object.
     \param[in] frame Sprite frame name.
     */
    DAVA_DEPRECATED(virtual void SetSpriteFrame(const FastName& frameName));
    /**
     \brief Sets draw type you want to use the control UIControlBackground object.
     \param[in] drawType Draw type to use for drawing.
     */
    DAVA_DEPRECATED(virtual void SetSpriteDrawType(UIControlBackground::eDrawType drawType));
    /**
     \brief Sets Sprite align you want to use for draw for the control UIControlBackground object.
     \param[in] drawAlign Sprite eAlign bit mask.
     */
    DAVA_DEPRECATED(virtual void SetSpriteAlign(int32 align));

    /**
     \brief Sets background what will be used for draw.
        Background is cloned inside control.
     \param[in] newBg control background you want to use for draw.
     */
    DAVA_DEPRECATED(void SetBackground(UIControlBackground* newBg));
    /**
     \brief Returns current background used for draw.
     \returns background used for draw.
     */
    DAVA_DEPRECATED(UIControlBackground* GetBackground() const);

    /**
     \brief Returns untransformed control rect.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control rect.
     */
    inline Rect GetRect() const;

    /**
     \brief Returns absolute untransformed control rect.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control rect.
     */
    Rect GetAbsoluteRect() const;

    /**
     \brief Sets the untransformed control rect.
     \param[in] rect new control rect.
     */
    virtual void SetRect(const Rect& rect);

    /**
     \brief Sets the untransformed control absolute rect.
     \param[in] rect new control absolute rect.
     */
    void SetAbsoluteRect(const Rect& rect);

    /**
     \brief Returns untransformed control position.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control position.
     */
    inline const Vector2& GetPosition() const;

    /**
     \brief Returns untransformed control position.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control absolute position.
     */
    Vector2 GetAbsolutePosition() const;

    /**
     \brief Sets the untransformed control position.
     \param[in] position new control position.
     */
    virtual void SetPosition(const Vector2& position);

    /**
     \brief Sets the untransformed control absolute position.
     \param[in] position new control absolute position.
     */
    void SetAbsolutePosition(const Vector2& position);

    /**
     \brief Returns untransformed control size.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control size.
     */
    inline const Vector2& GetSize() const;

    /**
     \brief Sets the untransformed control size.
     \param[in] newSize new control size.
     */
    virtual void SetSize(const Vector2& newSize);

    /**
     \brief Returns control pivot point.
     \returns control pivot point.
     */
    inline Vector2 GetPivotPoint() const;

    /**
     \brief Sets the control pivot point.
     \param[in] newPivot new control pivot point.
     */
    void SetPivotPoint(const Vector2& newPivot);

    /**
     \brief Returns control pivot.
     \returns control pivot.
     */
    inline const Vector2& GetPivot() const;

    /**
     \brief Sets the control pivot.
     \param[in] newPivot new control pivot.
     */
    void SetPivot(const Vector2& newPivot);

    /**
     \brief Returns control scale.
     \returns control scale.
     */
    inline const Vector2& GetScale() const;

    /**
     \brief Sets the control scale.
     \param[in] newScale new control scale.
     */
    inline void SetScale(const Vector2& newScale);

    /**
     \brief Returns actual control transformation and metrics.
     \returns control geometric data.
     */
    virtual const UIGeometricData& GetGeometricData() const;

    /**
     \brief Returns actual control local transformation and metrics.
     \returns control geometric data.
     */
    UIGeometricData GetLocalGeometricData() const;

    /**
     \brief Sets the scaled control rect.
        This method didn't apply any changes to the control size, but recalculate control scale.
     Warning, rectInAbsoluteCoordinates isn't properly works for now!
     \param[in] rect new control rect.
     */
    virtual void SetScaledRect(const Rect& rect, bool rectInAbsoluteCoordinates = false);

    /**
     \brief Returns control rotation angle in radians.
     \returns control angle in radians.
     */
    inline float32 GetAngle() const;
    inline float32 GetAngleInDegrees() const;

    /**
     \brief Sets contol rotation angle in radians.
        Control rotates around the pivot point.
     \param[in] angleInRad new control angle in radians.
     */
    virtual void SetAngle(float32 angleInRad);

    void SetAngleInDegrees(float32 angle);

    virtual Vector2 GetContentPreferredSize(const Vector2& constraints) const; // -1.0f means no constraint for axis
    virtual bool IsHeightDependsOnWidth() const;

    /**
     \brief Returns control visibility.
        Invisible controls don't process any inputs.
        Also for invisible controls didn't calls Draw() and DrawAfterChilds() methods.
     \returns control visibility.
     */
    inline bool GetVisibilityFlag() const;

    /**
     \brief Sets contol recursive visibility.
        Invisible controls don't process any inputs.
        Also for invisible controls didn't calls Draw() and DrawAfterChilds() methods.
     \param[in] isVisible new control visibility.
     */
    virtual void SetVisibilityFlag(bool isVisible);

    /**
     \brief Returns control input processing ability.
        Be ware! Base control processing inputs by default.
     \returns true if control pocessing inputs.
     */
    inline bool GetInputEnabled() const;

    /**
     \brief Sets contol input processing ability.
        If input is disabled control don't process any inputs. If input is disabled all inputs events would comes to the parent control.
        Please use input enabling/disabling for the single controls or forthe small parts of hierarchy.
        It's always better to add transparent control that covers all screen and would process all
        incoming inputs to prevent input processing for the all screen controls or for the large part of hierarchy.
     \param[in] isEnabled is control should process inputs?
     \param[in] hierarchic use true if you want to all control children change input ability.
     */
    virtual void SetInputEnabled(bool isEnabled, bool hierarchic = true);

    /**
     \brief Returns control enabling state.
        Disabled control don't process any inputs. But allows input processing for their children.
        Use this state only if you want change graphical representation of the control.
        Don't use this state for the disabling inputs for parts of the controls hierarchy!
        All controls is enabled by default.
     \returns true if control is disabled.
     */
    bool GetDisabled() const;

    /**
     \brief Sets the contol enabling/disabling.
        Disabled control don't process any inputs. But allows input processing for their children.
        Use this state only if you want change graphical representation of the control.
        Don't use this state for the disabling inputs for parts of the controls hierarchy!
        All controls is enabled by default.
     \param[in] isDisabled is control disabled?
     \param[in] hierarchic use true if you want to all control children change enabling/disabling.
     */
    virtual void SetDisabled(bool isDisabled, bool hierarchic = true);

    /**
     \brief Returns control selection state.
     \returns is control selected.
     */
    bool GetSelected() const;

    /**
     \brief Sets contol selection state.
        Selection state don't influence on any control activities.
     \param[in] isSelected is control selected?
     \param[in] hierarchic use true if you want to all control children change selection state.
     */
    virtual void SetSelected(bool isSelected, bool hierarchic = true);

    /**
     \brief Returns control clip contents state.
        Clip contents is disabled by default.
     \returns true if control rect clips draw and input areas of his children.
     */
    inline bool GetClipContents() const;
    /**
     \brief Sets clip contents state.
        If clip contents is enabled all incoming inputs for the control children processed only
        inside the control rect of parent. All children draw clips too.
        Clip contents is disabled by default.
     \param[in] isNeedToClipContents true if control should clips all children draw and input by his rect.
     */
    virtual void SetClipContents(bool isNeedToClipContents);

    /**
     \brief Returns control hover state.
        Only controlsa what processed inputs may be hovered.
     \returns control hover state is true if mouse placed over the control rect and no mous buttons is pressed.
     */
    bool GetHover() const;

    /**
     \brief Is exclusive input enabled.
        If control have exlusive input enabled and this control starts to process
        inputs. All inputs would be directed only to this control. But this control can
        process multiple number of inputs at a time.
        Exclusive input is disabled by default.
     \returns true if control supports exclusive input.
     */
    inline bool GetExclusiveInput() const;
    /**
     \brief Enables or disables control exclusive input.
        If control have exlusive input enabled and this control starts to process
        inputs. All inputs would be directed only to this control. But this control can
        process multiple number of inputs at a time.
        Exclusive input is disabled by default.
     \param[in] isExclusiveInput should control process inputs exclusively?
     \param[in] hierarchic use true if you want to all control children change exclusive input state.
     */
    virtual void SetExclusiveInput(bool isExclusiveInput, bool hierarchic = true);
    /**
     \brief Checks if control is multiple input enabled.
        If multiple input is enabled control can process all incoming inputs (Two or
        more touches for example). Otherwise control process only first incoming input.
        Multiply input is disabled by default.
     \returns true if control supports multyple inputs.
     */
    inline bool GetMultiInput() const;
    /**
     \brief Sets contol multi input processing.
        If multiple input is enabled control can process all incoming inputs (Two or
        more touches for example). Otherwise control process only first incoming input.
        Multiply input is disabled by default.
     \param[in] isMultiInput should control supports multiple inputs?
     \param[in] hierarchic use true if you want to all control children change multi nput support state.
     */
    virtual void SetMultiInput(bool isMultiInput, bool hierarchic = true);
    /**
    \brief Children will be sorted with predicate.
    Function uses stable sort, sets layout dirty flag and invalidates iteration.
    \param[in] predicate sorting predicate. All predicates for std::list<>::sort are allowed for this function too.
    */
    template <class T>
    inline void SortChildren(const T& predicate);
    /*
     \brief Sets the contol name.
        Later you can find control by this name.
     \param[in] _name new control name.
     */
    void SetName(const String& name_);
    void SetName(const FastName& name_);

    /**
     \brief Returns current name of the control.
     \returns control name.
     */
    inline const FastName& GetName() const;

    /**
     \brief Sets the contol tag.
     \param[in] tag new control tag.
     */
    void SetTag(int32 tag);

    /**
     \brief Returns current control tag.
     \returns control tag.
     */
    inline int32 GetTag() const;

    /**
     \brief Returns control with given name.
     \param[in] name requested control name.
     \param[in] recursive use true if you want fro recursive search.
     \returns first control with given name.
     */
    UIControl* FindByName(const String& name, bool recursive = true) const;
    UIControl* FindByName(const FastName& name, bool recursive = true) const;

    const UIControl* FindByPath(const String& path) const;
    UIControl* FindByPath(const String& path);

    template <class C>
    C FindByPath(const String& path) const
    {
        return DynamicTypeCheck<C>(FindByPath(path));
    }

    template <class C>
    C FindByPath(const String& path)
    {
        return DynamicTypeCheck<C>(FindByPath(path));
    }

    /**
     \brief Returns control state bit mask.
     \returns control state.
     */
    inline int32 GetState() const;
    /**
     \brief Sets control state bit mask.
        Try to not use this method manually.
     \param[in] state new control bit mask.
     */
    void SetState(int32 state);

    /**
     \brief Returns control parent.
     \returns if contorl hasn't parent returns NULL.
     */
    UIControl* GetParent() const;

    /**
     \brief Returns list of control children.
     \returns list of control children.
     */
    const List<UIControl*>& GetChildren() const;
    /**
     \brief Add control as a child.
        Children draws in the sequence of adding. If child has another parent
        this child removes from the parrent firstly.
     \param[in] control control to add.
     */
    virtual void AddControl(UIControl* control);
    /**
     \brief Removes control from the children list.
        If child isn't present in the method owners list nothin happens.
     \param[in] control control to remove.
     */
    virtual void RemoveControl(UIControl* control);
    /**
     \brief Remove this control from its parent, if any.
     */
    virtual void RemoveFromParent();
    /**
     \brief Removes all children from the control.
     */
    virtual void RemoveAllControls();
    /**
     \brief Brings given child front.
        This child will be drawn at the top of the control children.
        If child isn't present in the owners list nothin happens.
     \param[in] _control control to bring front.
     */
    virtual void BringChildFront(UIControl* _control);
    /**
     \brief Brings given child back.
        This child will be drawn at the bottom of the control children.
        If child isn't present in the owners list nothin happens.
     \param[in] _control control to bring back.
     */
    virtual void BringChildBack(UIControl* _control);
    /**
     \brief Inserts given child before the requested.
     \param[in] _control control to insert.
     \param[in] _belowThisChild control to insert before. If this control isn't present in the
        children list new child adds at the top of the list.
     */
    virtual void InsertChildBelow(UIControl* _control, UIControl* _belowThisChild);
    /**
     \brief Inserts given child after the requested.
     \param[in] _control control to insert.
     \param[in] _aboveThisChild control to insert after. If this control isn't present in the
     children list new child adds at the top of the list.
     */
    virtual void InsertChildAbove(UIControl* _control, UIControl* _aboveThisChild);
    /**
     \brief Sends given child before the requested.
        If one of the given children isn't present in the owners list nothin happens.
     \param[in] _control control to move.
     \param[in] _belowThisChild control to sends before.
     */
    virtual void SendChildBelow(UIControl* _control, UIControl* _belowThisChild);
    /**
     \brief Sends given child after the requested.
        If one of the given children isn't present in the owners list nothin happens.
     \param[in] _control control to move.
     \param[in] _aboveThisChild control to sends after.
     */
    virtual void SendChildAbove(UIControl* _control, UIControl* _aboveThisChild);

    /**
     \brief Adds callback message for the event trigger.
     \param[in] eventType event type you want to process.
     \param[in] msg message should be calld when the event triggered.
     */
    void AddEvent(int32 eventType, const Message& msg);
    /**
     \brief Removes callback message for the event trigger.
     \param[in] eventType event type you want to remove.
     \param[in] msg message to remove.
     \returns true if event is removed.
     */
    bool RemoveEvent(int32 eventType, const Message& msg);
    /**
     \brief Function to remove all events from event dispatcher.
     \returns true if we removed something, false if not
     */
    bool RemoveAllEvents();

    /**
     \brief Send given event to the all subscribed objects.
     \param[in] eventType event type you want to process.
     \param[in] uiEvent input event that triggered this control event.
     */
    void PerformEvent(int32 eventType, const UIEvent* uiEvent = nullptr);
    /**
     \brief Send given event with given user data to the all subscribed objects.
     \param[in] eventType event type you want to process.
     \param[in] callerData data you want to send to the all messages.
     \param[in] uiEvent input event that triggered this control event.
     */
    void PerformEventWithData(int32 eventType, void* callerData, const UIEvent* uiEvent = nullptr);

    /**
     \brief Creates the absoulutely identic copy of the control.
     \returns control copy.
     */
    virtual UIControl* Clone();

    RefPtr<UIControl> SafeClone();
    /**
     \brief Copies all contorl parameters from the sended control.
     \param[in] srcControl Source control to copy parameters from.
     */
    virtual void CopyDataFrom(UIControl* srcControl);

    //Animation helpers

    /**
     \brief Starts wait animation for the control.
     \param[in] time animation time.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* WaitAnimation(float32 time, int32 track = 0);
    /**
     \brief Starts move and size animation for the control.
     \param[in] rect New control position and size.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* MoveAnimation(const Rect& rect, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts move and scale animation for the control. Changing scale instead of size.
     \param[in] rect New control position and size.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* ScaledRectAnimation(const Rect& rect, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts scale animation for the control. Changing scale instead of size.
     \param[in] newSize New control size.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* ScaledSizeAnimation(const Vector2& newSize, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts control position animation.
     \param[in] _position New control position.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* PositionAnimation(const Vector2& _position, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts control size animation.
     \param[in] _size New control size.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* SizeAnimation(const Vector2& _size, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts control scale animation.
     \param[in] newScale New control scale.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* ScaleAnimation(const Vector2& newScale, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts control rotation angle animation.
     \param[in] newAngle New control rotation angle.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* AngleAnimation(float32 newAngle, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts input enabling switching animation. This animation changing control
        input enabling state on the next frame after the animation start.
     \param[in] touchable New input enabled value.
     \param[in] hierarhic Is value need to be changed in all coltrol children.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* TouchableAnimation(bool touchable, bool hierarhic = true, int32 track = 0);
    /**
     \brief Starts control disabling animation. This animation changing control
        disable state on the next frame after the animation start.
     \param[in] disabled New control disabling value.
     \param[in] hierarhic Is value need to be changed in all coltrol children.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* DisabledAnimation(bool disabled, bool hierarhic = true, int32 track = 0);
    /**
     \brief Starts control visible animation. This animation changing control visibility
        on the next frame after the animation start.
     \param[in] visible New control recursive visible value.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* VisibleAnimation(bool visible, int32 track = 0);
    /**
     \brief Starts control removation animation. This animation removes control from the parent
     on the next frame  after the animation start.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* RemoveControlAnimation(int32 track = 0);
    /**
     \brief Starts control color animation.
     \param[in] New control color.
     \param[in] animation time.
     \param[in] time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation* ColorAnimation(const Color& finalColor, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);

protected:
    void TouchableAnimationCallback(BaseObject* caller, void* param, void* callerData);
    void DisabledAnimationCallback(BaseObject* caller, void* param, void* callerData);
    void VisibleAnimationCallback(BaseObject* caller, void* param, void* callerData);
    void RemoveControlAnimationCallback(BaseObject* caller, void* param, void* callerData);

public:
    /**
     \brief enabling or disabling dbug draw for the control.
     \param[in] _debugDrawEnabled New debug draw value.
     \param[in] hierarchic Is value need to be changed in all coltrol children.
     */
    void SetDebugDraw(bool _debugDrawEnabled, bool hierarchic = false);
    void SetDebugDrawColor(const Color& color);
    const Color& GetDebugDrawColor() const;

    bool IsHiddenForDebug() const;
    void SetHiddenForDebug(bool hidden);

    /**
     \brief Set the draw pivot point mode for the control.
     \param[in] mode draw pivot point mode
     \param[in] hierarchic Is value need to be changed in all coltrol children.
     */
    void SetDrawPivotPointMode(eDebugDrawPivotMode mode, bool hierarchic = false);

public:
    /**
     \brief Calls on every frame to process controls drawing.
        Firstly this method calls Draw() for the curent control. When SystemDraw() called for the every control child.
        And at the end DrawAfterChilds() called for current control.
        Internal method used by ControlSystem.
        Can be overriden to adjust draw hierarchy.
     \param[in] geometricData Parent geometric data.
     */
    virtual void SystemDraw(const UIGeometricData& geometricData, const DAVA::UIControlBackground* parentBackground); // Internal method used by ControlSystem

    /**
     \brief set parent draw color into control
     \param[in] parentColor draw color of parent background.
     */
    virtual void SetParentColor(const Color& parentColor);

    /**
     \brief Calls on every input event. Calls SystemInput() for all control children.
        If no one of the children is processed input. Calls ProcessInput() for the current control.
        Internal method used by ControlSystem.
     \param[in] currentInput Input information.
     \returns true if control processed this input.
     */
    virtual bool SystemInput(UIEvent* currentInput);
    /**
     \brief Process incoming input and if it's necessary calls Input() method for the control.
        Internal method used by ControlSystem.
     \param[in] currentInput Input information.
     \returns true if control processed this input.
     */
    virtual bool SystemProcessInput(UIEvent* currentInput); // Internal method used by ControlSystem

    Function<bool(UIControl*, UIEvent*)> customSystemProcessInput;

    /**
     \brief Calls when input processd by control is cancelled.
        Internal method used by ControlSystem.
     \param[in] currentInput Input information.
     */
    virtual void SystemInputCancelled(UIEvent* currentInput);

    /**
     \brief Called when control is set as the hovered (by the mouse) control.
     Internal method used by ControlSystem. Can be overriden only by the people ho knows UI architecture.
     */
    virtual void SystemDidSetHovered();
    /**
     \brief Called when control is not a hovered (by the mouse) control.
     Internal method used by ControlSystem. Can be overriden only by the people ho knows UI architecture.
     */
    virtual void SystemDidRemoveHovered();

    /**
     \brief Called when control is set as the hovered (by the mouse) control.
     Can be overriden to implement start hoverig reaction.
     */
    virtual void DidSetHovered();
    /**
     \brief Called when control is not a hovered (by the mouse) control.
     Can be overriden to implement end hoverig reaction.
     */
    virtual void DidRemoveHovered();

    /**
     \brief Calls on every input event coming to control.
        Should be overriden to implement custom input reaction.
        During one input processing step into control may come more then one input event.
        For example: Pressing began event and pressing ended or five conituous mose move events etc.
        Called only if control inputEnable is true.
     \param[in] currentInput Input information.
     */
    virtual void Input(UIEvent* currentInput);
    /**
     \brief Calls when input processd by control is cancelled.
        Should be overriden to implement custom input cancelling reaction.
     \param[in] currentInput Input information.
     */
    virtual void InputCancelled(UIEvent* currentInput);
    /**
	 \brief Calls on every frame with frame delata time parameter.
            Works only with added UIUpdateComponent!
            Should be overriden to implement perframe functionality.
            Default realization is empty.
	 \param[in] timeElapsed Current frame time delta.
	 */
    virtual void Update(float32 timeElapsed);
    /**
     \brief Calls on every frame to draw control.
        Can be overriden to implement custom draw functionality.
        Default realization is drawing UIControlBackground with requested parameters.
     \param[in] geometricData Control geometric data.
     */
    virtual void Draw(const UIGeometricData& geometricData);
    /**
     \brief Calls on every frame with UIGeometricData after all children is drawed.
        Can be overriden to implement after children drawing.
        Default realization is empty.
     \param[in] geometricData Control geometric data.
     */
    virtual void DrawAfterChilds(const UIGeometricData& geometricData);

protected:
    enum class eViewState : int32
    {
        INACTIVE,
        ACTIVE,
        VISIBLE,
    };

    virtual void SystemVisible();
    virtual void SystemInvisible();

    virtual void OnVisible();
    virtual void OnInvisible();

    virtual void SystemActive();
    virtual void SystemInactive();

    virtual void OnActive();
    virtual void OnInactive();

    virtual void SystemScreenSizeChanged(const Rect& newFullScreenRect);
    virtual void OnScreenSizeChanged(const Rect& newFullScreenRect);

    void InvokeActive(eViewState parentViewState);
    void InvokeInactive();

    void InvokeVisible(eViewState parentViewState);
    void InvokeInvisible();

    void ChangeViewState(eViewState newViewState);

    void AddState(int32 state);
    void RemoveState(int32 state);

public:
    /**
     \brief Called when this control and his children are loaded.
     */
    virtual void LoadFromYamlNodeCompleted(){};

    /**
     \brief Returns control in hierarchy status.
     \returns True if control in view hierarchy for now.
     */
    bool IsActive() const;

    /**
     \brief Returns control on screen status.
     \returns True if control visible now.
     */
    bool IsVisible() const;
    /**
     \brief Returns point status realtive to control .
     \param[in] point Point to check.
     \param[in] expandWithFocus Is area should be expanded with focus.
     \returns True if inside the control rect.
     */
    virtual bool IsPointInside(const Vector2& point, bool expandWithFocus = false) const;

    virtual void SystemOnFocusLost();

    virtual void SystemOnFocused();

    virtual void OnFocusLost();

    virtual void OnFocused();

    virtual void OnTouchOutsideFocus();

    /// sets rect to match background sprite, also moves pivot point to center
    void SetSizeFromBg(bool pivotToCenter = true);

    virtual void UpdateLayout();
    virtual void OnSizeChanged();

    // Find the control by name and add it to the list, if found.
    bool AddControlToList(List<UIControl*>& controlsList, const String& controlName, bool isRecursive = false);

    void DumpInputs(int32 depthLevel);

    static void DumpControls(bool onlyOrphans);

private:
    FastName name;
    Vector2 pivot; //!<control pivot. Top left control corner by default.

    UIControl* parent;
    List<UIControl*> children;

    DAVA_DEPRECATED(bool isUpdated = false);
    DAVA_DEPRECATED(void SystemUpdate(float32 timeElapsed));
    // Need for old implementation of SystemUpdate.
    friend class UIScreenshoter;

public:
    //TODO: store geometric data in UIGeometricData
    Vector2 relativePosition; //!<position in the parent control.
    Vector2 size; //!<control size.

    Vector2 scale; //!<control scale. Scale relative to pivot point.
    float32 angle; //!<control rotation angle. Rotation around pivot point.

protected:
    float32 wheelSensitivity = 30.f;

    // boolean flags are grouped here to pack them together (see please DF-2149).
    bool exclusiveInput : 1;
    bool isInputProcessed : 1;
    bool visible : 1;
    bool hiddenForDebug : 1;
    bool clipContents : 1;
    bool debugDrawEnabled : 1;
    bool multiInput : 1;

    // Enable align options
    bool isIteratorCorrupted : 1;

    bool styleSheetDirty : 1;
    bool styleSheetInitialized : 1;
    bool layoutDirty : 1;
    bool layoutPositionDirty : 1;
    bool layoutOrderDirty : 1;

    int32 inputProcessorsCount;

    int32 currentInputID;
    int32 touchesInside;
    int32 totalTouches;

    mutable UIGeometricData tempGeometricData;

    EventDispatcher* eventDispatcher;

    Color debugDrawColor;

    eDebugDrawPivotMode drawPivotPointMode;

    void SetParent(UIControl* newParent);

    virtual ~UIControl();

    void RegisterInputProcessor();
    void RegisterInputProcessors(int32 processorsCount);
    void UnregisterInputProcessor();
    void UnregisterInputProcessors(int32 processorsCount);

    void DrawDebugRect(const UIGeometricData& geometricData, bool useAlpha = false);
    void DrawPivotPoint(const Rect& drawRect);

private:
    int32 tag = 0;
    eViewState viewState = eViewState::INACTIVE;
    int32 controlState;

    bool inputEnabled : 1;

    /* Components */
public:
    void AddComponent(UIComponent* component);
    void InsertComponentAt(UIComponent* component, uint32 index);
    void RemoveComponent(UIComponent* component);
    void RemoveComponent(uint32 componentType, uint32 index = 0);
    void RemoveAllComponents();

    UIComponent* GetComponent(uint32 componentType, uint32 index = 0) const;
    int32 GetComponentIndex(const UIComponent* component) const;
    UIComponent* GetOrCreateComponent(uint32 componentType, uint32 index = 0);

    template <class T>
    inline T* GetComponent(uint32 index = 0) const
    {
        return DynamicTypeCheck<T*>(GetComponent(T::C_TYPE, index));
    }
    template <class T>
    inline T* GetOrCreateComponent(uint32 index = 0)
    {
        return DynamicTypeCheck<T*>(GetOrCreateComponent(T::C_TYPE, index));
    }
    template <class T>
    inline uint32 GetComponentCount() const
    {
        return GetComponentCount(T::C_TYPE);
    }

    uint32 GetComponentCount() const;
    uint32 GetComponentCount(uint32 componentType) const;
    uint64 GetAvailableComponentFlags() const;

    const Vector<UIComponent*>& GetComponents();

private:
    Vector<UIComponent*> components;
    UIControlFamily* family;
    void RemoveComponent(const Vector<UIComponent*>::iterator& it);
    void UpdateFamily();
    /* Components */

    /* Styles */
public:
    void AddClass(const FastName& clazz);
    void RemoveClass(const FastName& clazz);
    bool HasClass(const FastName& clazz) const;
    void SetTaggedClass(const FastName& tag, const FastName& clazz);
    FastName GetTaggedClass(const FastName& tag) const;
    void ResetTaggedClass(const FastName& tag);

    String GetClassesAsString() const;
    void SetClassesFromString(const String& classes);

    const UIStyleSheetPropertySet& GetLocalPropertySet() const;
    void SetLocalPropertySet(const UIStyleSheetPropertySet& set);
    void SetPropertyLocalFlag(uint32 propertyIndex, bool value);

    const UIStyleSheetPropertySet& GetStyledPropertySet() const;
    void SetStyledPropertySet(const UIStyleSheetPropertySet& set);

    bool IsStyleSheetInitialized() const;
    void SetStyleSheetInitialized();

    bool IsStyleSheetDirty() const;
    void SetStyleSheetDirty();
    void ResetStyleSheetDirty();

    bool IsLayoutDirty() const;
    void SetLayoutDirty();
    void ResetLayoutDirty();

    bool IsLayoutPositionDirty() const;
    void SetLayoutPositionDirty();
    void ResetLayoutPositionDirty();

    bool IsLayoutOrderDirty() const;
    void SetLayoutOrderDirty();
    void ResetLayoutOrderDirty();

    UIControlPackageContext* GetPackageContext() const;
    UIControlPackageContext* GetLocalPackageContext() const;
    void SetPackageContext(UIControlPackageContext* packageContext);
    UIControl* GetParentWithContext() const;

private:
    UIStyleSheetClassSet classes;
    UIStyleSheetPropertySet localProperties;
    UIStyleSheetPropertySet styledProperties;
    RefPtr<UIControlPackageContext> packageContext;
    UIControl* parentWithContext;

    void PropagateParentWithContext(UIControl* newParentWithContext);
    /* Styles */

public:
    inline float32 GetWheelSensitivity() const;
    inline void SetWheelSensitivity(float32 newSens);

    // for introspection
    inline bool GetEnabled() const;
    inline void SetEnabledNotHierarchic(bool enabled);
    inline void SetSelectedNotHierarchic(bool enabled);
    inline void SetExclusiveInputNotHierarchic(bool enabled);
    inline bool GetNoInput() const;
    inline void SetNoInput(bool noInput);
    inline bool GetDebugDraw() const;
    inline void SetDebugDrawNotHierarchic(bool val);
};

inline Vector2 UIControl::GetPivotPoint() const
{
    return pivot * size;
}

inline const Vector2& UIControl::GetPivot() const
{
    return pivot;
}

inline const Vector2& UIControl::GetScale() const
{
    return scale;
}

inline void UIControl::SetScale(const Vector2& newScale)
{
    scale = newScale;
}

inline const Vector2& UIControl::GetSize() const
{
    return size;
}

inline const Vector2& UIControl::GetPosition() const
{
    return relativePosition;
}

inline float32 UIControl::GetAngle() const
{
    return angle;
}

inline float32 UIControl::GetAngleInDegrees() const
{
    return RadToDeg(angle);
}

inline const FastName& UIControl::GetName() const
{
    return name;
}

inline int32 UIControl::GetTag() const
{
    return tag;
}

inline Rect UIControl::GetRect() const
{
    return Rect(GetPosition() - GetPivotPoint(), GetSize());
}

inline bool UIControl::GetVisibilityFlag() const
{
    return visible;
}

inline bool UIControl::GetInputEnabled() const
{
    return inputEnabled;
}

inline bool UIControl::GetClipContents() const
{
    return clipContents;
}

inline bool UIControl::GetExclusiveInput() const
{
    return exclusiveInput;
}

inline bool UIControl::GetMultiInput() const
{
    return multiInput;
}

template <class T>
inline void UIControl::SortChildren(const T& predicate)
{
    children.sort(predicate); // std::stable_sort and std::sort are not allowed for list

    isIteratorCorrupted = true;
    SetLayoutOrderDirty();
}

inline int32 UIControl::GetState() const
{
    return controlState;
}

inline bool UIControl::GetEnabled() const
{
    return !GetDisabled();
}

inline void UIControl::SetEnabledNotHierarchic(bool enabled)
{
    SetDisabled(!enabled, false);
}

inline void UIControl::SetSelectedNotHierarchic(bool selected)
{
    SetSelected(selected, false);
}

inline void UIControl::SetExclusiveInputNotHierarchic(bool enabled)
{
    SetExclusiveInput(enabled, false);
}

inline bool UIControl::GetNoInput() const
{
    return !GetInputEnabled();
}

inline void UIControl::SetNoInput(bool noInput)
{
    SetInputEnabled(!noInput, false);
}

inline bool UIControl::GetDebugDraw() const
{
    return debugDrawEnabled;
}

inline void UIControl::SetDebugDrawNotHierarchic(bool val)
{
    SetDebugDraw(val, false);
}

inline float32 UIControl::GetWheelSensitivity() const
{
    return wheelSensitivity;
}
inline void UIControl::SetWheelSensitivity(float32 newSens)
{
    wheelSensitivity = newSens;
}

inline bool UIControl::IsLayoutDirty() const
{
    return layoutDirty;
}

inline bool UIControl::IsLayoutPositionDirty() const
{
    return layoutPositionDirty;
}

inline bool UIControl::IsLayoutOrderDirty() const
{
    return layoutOrderDirty;
}
};
