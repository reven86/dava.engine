
#include "EditorSystems/EditorCanvas.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "Modules/DocumentsModule/Private/EditorCanvasData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>

#include <Engine/Engine.h>
#include <UI/UIScreenManager.h>

using namespace DAVA;

EditorCanvas::EditorCanvas(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(parent, accessor)
{
    movableControl = systemsManager->GetScalableControl();
    systemsManager->contentSizeChanged.Connect(this, &EditorCanvas::OnContentSizeChanged);

    predefinedScales = { 0.25f, 0.33f, 0.50f, 0.67f, 0.75f,
                         0.90f, 1.00f, 1.10f, 1.25f, 1.50f, 1.75f, 2.00f,
                         2.50f, 3.00f, 4.00f, 5.00f, 6.00f, 7.00f, 8.00f };
}

EditorCanvas::~EditorCanvas() = default;

void EditorCanvas::AdjustScale(float32 newScale, const Vector2& mousePos)
{
    newScale = DAVA::Clamp(newScale, minScale, maxScale);
    if (scale == newScale)
    {
        return;
    }
    Vector2 oldPos = position;
    float32 oldScale = scale;
    scale = newScale;
    UpdateContentSize();
    scaleChanged.Emit(scale);
    movableControl->SetScale(Vector2(scale, scale));

    if (oldScale == 0.0f || viewSize.dx <= 0.0f || viewSize.dy <= 0.0f)
    {
        SetPosition(Vector2(0.0f, 0.0f));
        return;
    }

    Vector2 absPosition = oldPos / oldScale;
    Vector2 deltaMousePos = mousePos * (1.0f - newScale / oldScale);
    Vector2 newPosition(absPosition.x * scale - deltaMousePos.x, absPosition.y * scale - deltaMousePos.y);

    newPosition.x = Clamp(newPosition.x, 0.0f, (size - viewSize).dx);
    newPosition.y = Clamp(newPosition.y, 0.0f, (size - viewSize).dy);
    SetPosition(newPosition);
}

Vector2 EditorCanvas::GetSize() const
{
    return size;
}

Vector2 EditorCanvas::GetViewSize() const
{
    return viewSize;
}

Vector2 EditorCanvas::GetPosition() const
{
    return position;
}

float32 EditorCanvas::GetScale() const
{
    return scale;
}

float32 EditorCanvas::GetMinScale() const
{
    return minScale;
}

float32 EditorCanvas::GetMaxScale() const
{
    return maxScale;
}

Vector2 EditorCanvas::GetMinimumPos() const
{
    return Vector2(0.0f, 0.0f);
}

Vector2 EditorCanvas::GetMaximumPos() const
{
    Vector2 maxPos = size - viewSize;
    Vector2 minPos = GetMinimumPos();
    return Vector2(Max(maxPos.x, minPos.x), Max(maxPos.y, minPos.y));
}

void EditorCanvas::UpdateContentSize()
{
    using namespace TArc;

    Vector2 marginsSize(margin * 2.0f, margin * 2.0f);
    size = contentSize * scale + marginsSize;
    sizeChanged.Emit(size);

    Vector2 sizeDiff = (size - viewSize) / 2.0f;

    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext != nullptr)
    {
        EditorCanvasData* data = activeContext->GetData<EditorCanvasData>();

        //we select big control after small control was selected
        if ((data->needCentralizeX && sizeDiff.dx > 0.0f)
            || (data->needCentralizeY && sizeDiff.dy > 0.0f))
        {
            Vector2 newPosition(data->needCentralizeX ? sizeDiff.dx : position.x,
                                data->needCentralizeY ? sizeDiff.dy : position.y);
            SetPosition(newPosition);
        }
        else if (data->canvasPosition == EditorCanvasData::invalidPosition)
        {
            SetPosition(GetMaximumPos() / 2.0f);
        }
        else
        {
            SetPosition(data->canvasPosition);
        }
        data->needCentralizeX = size.dx < viewSize.dx;
        data->needCentralizeY = size.dy < viewSize.dy;
    }
    UpdatePosition();
}

void EditorCanvas::SetScale(float32 arg)
{
    if (scale != arg)
    {
        AdjustScale(arg, Vector2(viewSize.dx / 2.0f, viewSize.dy / 2.0f)); //cursor at center of view
    }
}

void EditorCanvas::OnViewSizeChanged(DAVA::uint32 width, DAVA::uint32 height)
{
    Vector2 viewSize_(static_cast<float32>(width), static_cast<float32>(height));
    if (viewSize_ != viewSize)
    {
        viewSize = viewSize_;
        UpdatePosition();
    }
}

void EditorCanvas::SetPosition(const Vector2& position_)
{
    using namespace TArc;
    Vector2 minPos = GetMinimumPos();
    Vector2 maxPos = GetMaximumPos();
    Vector2 fixedPos(Clamp(position_.x, minPos.x, maxPos.x),
                     Clamp(position_.y, minPos.y, maxPos.y));

    if (fixedPos != position)
    {
        position = fixedPos;
        UpdatePosition();
        positionChanged.Emit(position);

        DataContext* activeContext = accessor->GetActiveContext();
        if (activeContext == nullptr)
        {
            return;
        }
        EditorCanvasData* data = activeContext->GetData<EditorCanvasData>();
        data->canvasPosition = position;
    }
}

const Vector<float32>& EditorCanvas::GetPredefinedScales() const
{
    return predefinedScales;
}

void EditorCanvas::UpdatePosition()
{
    DVASSERT(nullptr != movableControl);
    Vector2 offset = (size - viewSize) / 2.0f;

    if (offset.dx > 0.0f)
    {
        offset.dx = position.x;
    }
    if (offset.dy > 0.0f)
    {
        offset.dy = position.y;
    }

    offset -= Vector2(margin, margin);
    Vector2 counterPosition(-offset.dx, -offset.dy);
    movableControl->SetPosition(counterPosition);

    nestedControlPositionChanged.Emit(counterPosition);
}

bool EditorCanvas::CanProcessInput(UIEvent* currentInput) const
{
    if ((currentInput->device & eInputDevices::CLASS_POINTER) == eInputDevices::UNKNOWN)
    {
        return false;
    }
    if (currentInput->phase == UIEvent::Phase::WHEEL || currentInput->phase == UIEvent::Phase::GESTURE)
    {
        return true;
    }
    return (systemsManager->GetDragState() == EditorSystemsManager::DragScreen &&
            (currentInput->mouseButton == eMouseButtons::LEFT || currentInput->mouseButton == eMouseButtons::MIDDLE));
}

void EditorCanvas::ProcessInput(UIEvent* currentInput)
{
    if (accessor->GetActiveContext() == nullptr)
    {
        return;
    }
    if (currentInput->device == eInputDevices::TOUCH_PAD)
    {
        if (currentInput->phase == UIEvent::Phase::GESTURE)
        {
            const UIEvent::Gesture& gesture = currentInput->gesture;
            if (gesture.dx != 0.0f || gesture.dy != 0.0f)
            {
                SetPosition(GetPosition() - Vector2(gesture.dx, gesture.dy));
            }
            else if (gesture.magnification != 0.0f)
            {
                AdjustScale(scale + gesture.magnification, currentInput->physPoint);
            }
        }
    }
    else if (currentInput->device == eInputDevices::MOUSE)
    {
        if (currentInput->phase == UIEvent::Phase::WHEEL)
        {
            if ((currentInput->modifiers & (eModifierKeys::CONTROL | eModifierKeys::COMMAND)) != eModifierKeys::NONE)
            {
                int32 ticksCount = static_cast<int32>(currentInput->wheelDelta.y);
                float scale = GetScaleFromWheelEvent(ticksCount);
                AdjustScale(scale, currentInput->physPoint);
            }
            else
            {
                Vector2 position = GetPosition();
                Vector2 additionalPos(currentInput->wheelDelta.x, currentInput->wheelDelta.y);
                additionalPos *= GetViewSize();
//custom delimiter to scroll widget by little chunks of visible area
#if defined(__DAVAENGINE_MACOS__)
                //on the OS X platform wheelDelta depend on scrolling speed
                static const float wheelDelta = 0.002f;
#elif defined(__DAVAENGINE_WIN32__)
                static const float wheelDelta = 0.1f;
#endif //platform
                Vector2 newPosition = position - additionalPos * wheelDelta;
                SetPosition(newPosition);
            }
        }
        else
        {
            Vector2 delta = systemsManager->GetMouseDelta();
            SetPosition(position - delta);
        }
    }
}

EditorSystemsManager::eDragState EditorCanvas::RequireNewState(UIEvent* currentInput)
{
    Function<void(UIEvent*, bool)> setMouseButtonOnInput = [this](const UIEvent* currentInput, bool value) {
        if (currentInput->mouseButton == eMouseButtons::MIDDLE)
        {
            isMouseMidButtonPressed = value;
        }
    };
    if (currentInput->device == eInputDevices::MOUSE)
    {
        if (currentInput->phase == UIEvent::Phase::BEGAN)
        {
            setMouseButtonOnInput(currentInput, true);
        }
        else if (currentInput->phase == UIEvent::Phase::ENDED)
        {
            setMouseButtonOnInput(currentInput, false);
        }
    }
    else if (currentInput->device == eInputDevices::KEYBOARD && currentInput->key == Key::SPACE)
    {
        //we cant use isKeyPressed here, because DAVA update keyboard state after sending Key_Up event
        //if we will press key on dock widget and hold it -> DAVA will receive only KEY_REPEAT event without KEY_DOWN
        if (currentInput->phase == UIEvent::Phase::KEY_DOWN || currentInput->phase == UIEvent::Phase::KEY_DOWN_REPEAT)
        {
            isSpacePressed = true;
        }
        else if (currentInput->phase == UIEvent::Phase::KEY_UP)
        {
            isSpacePressed = false;
        }
    }
    bool inDragScreenState = isMouseMidButtonPressed || isSpacePressed;
    return inDragScreenState ? EditorSystemsManager::DragScreen : EditorSystemsManager::NoDrag;
}

void EditorCanvas::OnContentSizeChanged(const DAVA::Vector2& size)
{
    contentSize = size;
    UpdateContentSize();
}

float32 EditorCanvas::GetScaleFromWheelEvent(int32 ticksCount) const
{
    if (ticksCount > 0)
    {
        return GetNextScale(ticksCount);
    }
    else if (ticksCount < 0)
    {
        return GetPreviousScale(ticksCount);
    }
    return scale;
}

float32 EditorCanvas::GetNextScale(int32 ticksCount) const
{
    auto iter = std::upper_bound(predefinedScales.begin(), predefinedScales.end(), scale);
    if (iter == predefinedScales.end())
    {
        return scale;
    }
    ticksCount--;
    int32 distance = static_cast<int32>(std::distance(iter, predefinedScales.end()));
    ticksCount = std::min(distance, ticksCount);
    std::advance(iter, ticksCount);
    return iter != predefinedScales.end() ? *iter : predefinedScales.back();
}

float32 EditorCanvas::GetPreviousScale(int32 ticksCount) const
{
    auto iter = std::lower_bound(predefinedScales.begin(), predefinedScales.end(), scale);
    if (iter == predefinedScales.end())
    {
        return scale;
    }
    int32 distance = static_cast<int32>(std::distance(iter, predefinedScales.begin()));
    ticksCount = std::max(ticksCount, distance);
    std::advance(iter, ticksCount);
    return *iter;
}
