#include "UI/UISlider.h"
#include "UI/UIButton.h"
#include "Render/RenderHelper.h"
#include "Base/ObjectFactory.h"
#include "Utils/Utils.h"
#include "Core/Core.h"
#include "UI/UIEvent.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA
{
// Use these names for children buttons to define UISlider in .yaml
static const FastName UISLIDER_THUMB_SPRITE_CONTROL_NAME("thumbSpriteControl");
static const FastName UISLIDER_MIN_SPRITE_CONTROL_NAME("minSpriteControl");
static const FastName UISLIDER_MAX_SPRITE_CONTROL_NAME("maxSpriteControl");

UISlider::UISlider(const Rect& rect)
    : UIControl(rect)
    , minBackground(NULL)
    , maxBackground(NULL)
    , thumbButton(NULL)
{
    SetInputEnabled(true, false);
    isEventsContinuos = true;

    leftInactivePart = 0;
    rightInactivePart = 0;
    minValue = 0.0f;
    maxValue = 1.0f;
    currentValue = 0.5f;

    minBackground = new UIControlBackground();
    maxBackground = new UIControlBackground();
    InitThumb();
}

void UISlider::InitThumb()
{
    thumbButton = new UIControl(Rect(0, 0, 40.f, 40.f));
    thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
    AddControl(thumbButton);

    thumbButton->SetInputEnabled(false);
    thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->SetPivot(Vector2(0.5f, 0.5f));

    SetValue(currentValue);
}

void UISlider::InitInactiveParts(Sprite* spr)
{
    if (NULL == spr)
    {
        return;
    }

    leftInactivePart = rightInactivePart = static_cast<int32>((spr->GetWidth() / 2.0f));
}

void UISlider::SetThumb(UIControl* newThumb)
{
    if (thumbButton == newThumb)
    {
        return;
    }

    RemoveControl(thumbButton);
    SafeRelease(thumbButton);

    thumbButton = SafeRetain(newThumb);
    thumbButton->SetName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
    thumbButton->SetInputEnabled(false);

    thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->SetPivot(Vector2(0.5f, 0.5f));

    UIControl::AddControl(thumbButton);

    SetValue(currentValue);
}

UISlider::~UISlider()
{
    SafeRelease(minBackground);
    SafeRelease(maxBackground);
    SafeRelease(thumbButton);
}

void UISlider::RecalcButtonPos()
{
    if (thumbButton)
    {
        thumbButton->relativePosition.x = Interpolation::Linear(static_cast<float32>(leftInactivePart), size.x - rightInactivePart, minValue, currentValue, maxValue);
        thumbButton->relativePosition.y = GetSize().y / 2; // thumb button pivot point is on center.
    }
}

void UISlider::SyncThumbWithSprite()
{
    RecalcButtonPos();
}

void UISlider::SetValue(float32 value)
{
    bool needSendEvent = !FLOAT_EQUAL(currentValue, value);
    currentValue = value;
    RecalcButtonPos();

    if (needSendEvent)
    {
        PerformEvent(EVENT_VALUE_CHANGED);
    }
}

void UISlider::SetMinValue(float32 value)
{
    minValue = value;
    if (currentValue < minValue)
    {
        SetValue(minValue);
    }
    else
    {
        RecalcButtonPos();
    }
}

void UISlider::SetMaxValue(float32 value)
{
    maxValue = value;
    if (currentValue > maxValue)
    {
        SetValue(maxValue);
    }
    else
    {
        RecalcButtonPos();
    }
}

void UISlider::SetMinMaxValue(float32 _minValue, float32 _maxValue)
{
    minValue = _minValue;
    maxValue = _maxValue;

    if (currentValue < minValue)
    {
        SetValue(minValue);
    }
    else if (currentValue > maxValue)
    {
        SetValue(maxValue);
    }
    else
    {
        RecalcButtonPos();
    }
}

void UISlider::AddControl(UIControl* control)
{
    // Synchronize the pointers to the thumb each time new control is added.
    UIControl::AddControl(control);

    if (control->GetName() == UISLIDER_THUMB_SPRITE_CONTROL_NAME && thumbButton != control)
    {
        SafeRelease(thumbButton);
        thumbButton = SafeRetain(control);
    }
}

void UISlider::RemoveControl(UIControl* control)
{
    if (control == thumbButton)
    {
        SafeRelease(thumbButton);
    }

    UIControl::RemoveControl(control);
}

void UISlider::Input(UIEvent* currentInput)
{
    // not supported for now.
    if (UIEvent::Phase::WHEEL == currentInput->phase || UIEvent::Phase::MOVE == currentInput->phase || UIEvent::Phase::CHAR == currentInput->phase || UIEvent::Phase::KEY_DOWN == currentInput->phase || UIEvent::Phase::KEY_UP == currentInput->phase || UIEvent::Phase::KEY_DOWN_REPEAT == currentInput->phase || UIEvent::Phase::CHAR_REPEAT == currentInput->phase || UIEvent::Phase::ERROR == currentInput->phase || UIEvent::Phase::JOYSTICK == currentInput->phase)
    {
        return;
    }

    const Rect& absRect = GetGeometricData().GetUnrotatedRect();
    //absTouchPoint = currentInput->point;

    relTouchPoint = currentInput->point;
    relTouchPoint -= absRect.GetPosition();

    float oldVal = currentValue;
    currentValue = Interpolation::Linear(minValue, maxValue, static_cast<float32>(leftInactivePart), relTouchPoint.x, size.x - static_cast<float32>(rightInactivePart));

    if (currentValue < minValue)
    {
        currentValue = minValue;
    }
    if (currentValue > maxValue)
    {
        currentValue = maxValue;
    }

    if (isEventsContinuos) // if continuos events
    {
        if (oldVal != currentValue)
        {
            PerformEventWithData(EVENT_VALUE_CHANGED, currentInput);
        }
    }
    else if (currentInput->phase == UIEvent::Phase::ENDED)
    {
        /* if not continuos always perform event because last move position almost always the same as end pos */
        PerformEventWithData(EVENT_VALUE_CHANGED, currentInput);
    }

    RecalcButtonPos();
    currentInput->SetInputHandledType(UIEvent::INPUT_HANDLED_HARD); // Drag is handled - see please DF-2508.
}

void UISlider::Draw(const UIGeometricData& geometricData)
{
    const Rect& aRect = thumbButton->GetGeometricData().GetUnrotatedRect();
    float32 clipPointAbsolute = aRect.x + aRect.dx * 0.5f;

    Rect fullVirtualScreen = VirtualCoordinatesSystem::Instance()->GetFullScreenVirtualRect();
    float32 screenXMin = fullVirtualScreen.x;
    float32 screenXMax = fullVirtualScreen.x + fullVirtualScreen.dx;
    float32 screenYMin = 0.f;
    float32 screenYMax = static_cast<float32>(VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy);

    if (minBackground)
    {
        minBackground->SetParentColor(GetBackground()->GetDrawColor());
        RenderSystem2D::Instance()->PushClip();
        RenderSystem2D::Instance()->IntersectClipRect(Rect(screenXMin, screenYMin, clipPointAbsolute - screenXMin, screenYMax));
        minBackground->Draw(geometricData);
        RenderSystem2D::Instance()->PopClip();
    }
    if (maxBackground)
    {
        maxBackground->SetParentColor(GetBackground()->GetDrawColor());
        RenderSystem2D::Instance()->PushClip();
        RenderSystem2D::Instance()->IntersectClipRect(Rect(clipPointAbsolute, screenYMin, screenXMax - clipPointAbsolute, screenYMax));
        maxBackground->Draw(geometricData);
        RenderSystem2D::Instance()->PopClip();
    }

    if (!minBackground && !maxBackground)
    {
        UIControl::Draw(geometricData);
    }
}

void UISlider::SetSize(const DAVA::Vector2& newSize)
{
    UIControl::SetSize(newSize);
    RecalcButtonPos();
}

void UISlider::LoadFromYamlNodeCompleted()
{
    AttachToSubcontrols();
    SyncThumbWithSprite();
}

UISlider* UISlider::Clone()
{
    UISlider* t = new UISlider(GetRect());
    t->CopyDataFrom(this);
    return t;
}

void UISlider::CopyDataFrom(UIControl* srcControl)
{
    RemoveControl(thumbButton);
    SafeRelease(thumbButton);

    UIControl::CopyDataFrom(srcControl);
    UISlider* t = static_cast<UISlider*>(srcControl);

    isEventsContinuos = t->isEventsContinuos;

    leftInactivePart = t->leftInactivePart;
    rightInactivePart = t->rightInactivePart;

    minValue = t->minValue;
    maxValue = t->maxValue;

    currentValue = t->currentValue;

    SafeRelease(minBackground);
    if (t->minBackground)
    {
        minBackground = t->minBackground->Clone();
    }

    SafeRelease(maxBackground);
    if (t->maxBackground)
    {
        maxBackground = t->maxBackground->Clone();
    }

    relTouchPoint = t->relTouchPoint;
}

void UISlider::AttachToSubcontrols()
{
    if (!thumbButton)
    {
        thumbButton = FindByName(UISLIDER_THUMB_SPRITE_CONTROL_NAME);
        DVASSERT(thumbButton);
        thumbButton->Retain();
    }

    InitInactiveParts(thumbButton->GetBackground()->GetSprite());
}

int32 UISlider::GetBackgroundComponentsCount() const
{
    return BACKGROUND_COMPONENTS_COUNT;
}

UIControlBackground* UISlider::GetBackgroundComponent(int32 index) const
{
    switch (index)
    {
    case 0:
        return GetBackground();

    case 1:
        return minBackground;

    case 2:
        return maxBackground;

    default:
        DVASSERT(false);
        return NULL;
    }
}

UIControlBackground* UISlider::CreateBackgroundComponent(int32 index) const
{
    DVASSERT(0 <= index && index < BACKGROUND_COMPONENTS_COUNT);
    return new UIControlBackground();
}

void UISlider::SetBackgroundComponent(int32 index, UIControlBackground* bg)
{
    DVASSERT(false);
}

String UISlider::GetBackgroundComponentName(int32 index) const
{
    DVASSERT(0 <= index && index < BACKGROUND_COMPONENTS_COUNT);
    static const String names[BACKGROUND_COMPONENTS_COUNT] = { "Background", "min", "max" };
    return names[index];
}

} // ns
