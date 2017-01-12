#include "EditorSystems/EditorCanvas.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "UI/UIScreenManager.h"

using namespace DAVA;

EditorCanvas::EditorCanvas(UIControl* movableControl_, EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , movableControl(movableControl_)
{
    systemsManager->contentSizeChanged.Connect(this, &EditorCanvas::OnContentSizeChanged);
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
    UpdateCanvasContentSize();
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

    newPosition.x = Clamp(newPosition.x, 0.0f, (canvasSize - viewSize).dx);
    newPosition.y = Clamp(newPosition.y, 0.0f, (canvasSize - viewSize).dy);
    SetPosition(newPosition);
}

Vector2 EditorCanvas::GetCanvasSize() const
{
    return canvasSize;
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
    return canvasSize - viewSize;
}

void EditorCanvas::UpdateCanvasContentSize()
{
    Vector2 marginsSize(Margin * 2.0f, Margin * 2.0f);
    canvasSize = contentSize * scale + marginsSize;
    UpdatePosition();
    canvasSizeChanged.Emit(canvasSize);
}

void EditorCanvas::SetScale(float32 arg)
{
    if (scale != arg)
    {
        AdjustScale(arg, Vector2(viewSize.dx / 2.0f, viewSize.dy / 2.0f)); //cursor at center of view
    }
}

void EditorCanvas::SetViewSize(const Vector2& viewSize_)
{
    if (viewSize_ != viewSize)
    {
        viewSize = viewSize_;
        int32 width = static_cast<int32>(viewSize.dx);
        int32 height = static_cast<int32>(viewSize.dy);
        VirtualCoordinatesSystem* vcs = UIControlSystem::Instance()->vcs;

        vcs->UnregisterAllAvailableResourceSizes();
        vcs->SetVirtualScreenSize(width, height);
        vcs->RegisterAvailableResourceSize(width, height, "Gfx");
        vcs->RegisterAvailableResourceSize(width, height, "Gfx2");

        UIScreenManager::Instance()->GetScreen()->SetSize(viewSize);
        UpdatePosition();
        viewSizeChanged.Emit(viewSize);
    }
}

void EditorCanvas::SetViewSize(DAVA::uint32 width, DAVA::uint32 height)
{
    Vector2 size(static_cast<float32>(width), static_cast<float32>(height));
    SetViewSize(size);
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
        positionChanged.Emit(position);
    }
}

void EditorCanvas::UpdatePosition()
{
    DVASSERT(nullptr != movableControl);
    Vector2 offset = (canvasSize - viewSize) / 2.0f;

    if (offset.dx > 0.0f)
    {
        offset.dx = position.x;
    }
    if (offset.dy > 0.0f)
    {
        offset.dy = position.y;
    }
    offset -= Vector2(Margin, Margin);
    Vector2 position(-offset.dx, -offset.dy);
    movableControl->SetPosition(position);

    nestedControlPositionChanged.Emit(position);
}

bool EditorCanvas::CanProcessInput(DAVA::UIEvent* currentInput) const
{
    return systemsManager->GetDragState() == EditorSystemsManager::DragScreen;
}

void EditorCanvas::ProcessInput(UIEvent* currentInput)
{
    DVASSERT(currentInput->phase == UIEvent::Phase::DRAG);
    Vector2 delta = systemsManager->GetMouseDelta();
    SetPosition(position + delta);
}

EditorSystemsManager::eDragState EditorCanvas::RequireNewState(UIEvent* currentInput)
{
    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();

    bool isSpacePressed = keyboard.IsKeyPressed(Key::SPACE);
    bool isMouseMidButtonPressed = currentInput->mouseButton == eMouseButtons::MIDDLE;
    bool isMouseLeftButtonPressed = currentInput->mouseButton == eMouseButtons::LEFT;

    bool inDragScreenState = isMouseMidButtonPressed || (isMouseLeftButtonPressed && isSpacePressed);
    return inDragScreenState ? EditorSystemsManager::DragScreen : EditorSystemsManager::NoDrag;
}

void EditorCanvas::OnContentSizeChanged(const DAVA::Vector2& size)
{
    contentSize = size;
    UpdateCanvasContentSize();
}
