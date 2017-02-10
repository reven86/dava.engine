#include "EditorSystems/EditorCanvas.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "Engine/Engine.h"

#include "UI/UIScreenManager.h"

using namespace DAVA;

EditorCanvas::EditorCanvas(UIControl* movableControl_, EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , movableControl(movableControl_)
{
    systemsManager->contentSizeChanged.Connect(this, &EditorCanvas::OnContentSizeChanged);
    systemsManager->viewSizeChanged.Connect(this, &EditorCanvas::OnViewSizeChanged);
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
    return size - viewSize;
}

void EditorCanvas::UpdateContentSize()
{
    Vector2 marginsSize(margin * 2.0f, margin * 2.0f);
    size = contentSize * scale + marginsSize;
    UpdatePosition();
    sizeChanged.Emit(size);
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
    Vector2 minPos = GetMinimumPos();
    Vector2 maxPos = GetMaximumPos();
    Vector2 fixedPos(Clamp(position_.x, minPos.x, maxPos.x),
                     Clamp(position_.y, minPos.y, maxPos.y));
    if (fixedPos != position)
    {
        position = fixedPos;
        UpdatePosition();
        ositionChanged.Emit(position);
    }
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
    Vector2 position(-offset.dx, -offset.dy);
    movableControl->SetPosition(position);

    nestedControlPositionChanged.Emit(position);
}

bool EditorCanvas::CanProcessInput(DAVA::UIEvent* currentInput) const
{
    return systemsManager->GetDragState() == EditorSystemsManager::DragScreen
    && currentInput->device == eInputDevices::MOUSE
    && (currentInput->mouseButton == eMouseButtons::LEFT || currentInput->mouseButton == eMouseButtons::MIDDLE);
}

void EditorCanvas::ProcessInput(UIEvent* currentInput)
{
    Vector2 delta = systemsManager->GetMouseDelta();
    SetPosition(position - delta);
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
