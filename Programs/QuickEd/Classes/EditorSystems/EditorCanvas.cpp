#include "EditorSystems/EditorCanvas.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "UI/UIScreenManager.h"

using namespace DAVA;

EditorCanvas::EditorCanvas(EditorSystemsManager *parent)
    : BaseEditorSystem(parent)
    , backgroundControl(new UIControl)
{
    backgroundControl->SetName(FastName("Background control of scroll area controller"));
    ScopedPtr<UIScreen> davaUIScreen(new UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(Color(0.3f, 0.3f, 0.3f, 1.0f));
    UIScreenManager::Instance()->RegisterScreen(0, davaUIScreen);
    UIScreenManager::Instance()->SetFirst(0);

    UIScreenManager::Instance()->GetScreen()->AddControl(backgroundControl);
}

EditorCanvas::~EditorCanvas()
{
    UIScreenManager::Instance()->ResetScreen();
}

void EditorCanvas::SetNestedControl(UIControl* arg)
{
    if (nullptr != nestedControl)
    {
        backgroundControl->RemoveControl(nestedControl);
    }
    nestedControl = arg;
    if (nullptr != nestedControl)
    {
        backgroundControl->AddControl(nestedControl);
        UpdateCanvasContentSize();
    }
}

void EditorCanvas::SetMovableControl(UIControl* arg)
{
    if (arg != movableControl)
    {
        movableControl = arg;
        UpdatePosition();
    }
}

void EditorCanvas::AdjustScale(float newScale, const Vector2& mousePos)
{
    newScale = fmax(minScale, newScale);
    newScale = fmin(maxScale, newScale); //crop scale to 800
    if (scale == newScale)
    {
        return;
    }
    Vector2 oldPos = position;
    float32 oldScale = scale;
    scale = newScale;
    UpdateCanvasContentSize();
    scaleChanged.Emit(scale);

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
    Vector2 contentSize;
    if (nullptr != nestedControl)
    {
        const UIGeometricData& gd = nestedControl->GetGeometricData();

        contentSize = gd.GetAABBox().GetSize() * scale;
    }
    Vector2 marginsSize(Margin * 2.0f, Margin * 2.0f);
    Vector2 tmpSize = contentSize + marginsSize;
    backgroundControl->SetSize(tmpSize);
    canvasSize = Vector2(tmpSize.dx, tmpSize.dy);
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

void EditorCanvas::SetViewSize(int32 width, int32 height)
{
    SetViewSize(Vector2(width, height));
}

void EditorCanvas::SetViewSize(const Vector2& viewSize_)
{
    if (viewSize_ != viewSize)
    {
        viewSize = viewSize_;

        VirtualCoordinatesSystem* vcs = UIControlSystem::Instance()->vcs;

        vcs->UnregisterAllAvailableResourceSizes();
        vcs->SetVirtualScreenSize(viewSize.dx, viewSize.dy);
        vcs->RegisterAvailableResourceSize(viewSize.dx, viewSize.dy, "Gfx");
        vcs->RegisterAvailableResourceSize(viewSize.dx, viewSize.dy, "Gfx2");

        Vector2 newSize(viewSize_.dx, viewSize_.dy);
        UIScreenManager::Instance()->GetScreen()->SetSize(newSize);
        UpdatePosition();
        viewSizeChanged.Emit(viewSize);
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
        positionChanged.Emit(position);
    }
}

void EditorCanvas::UpdatePosition()
{
    if (nullptr != movableControl)
    {
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

        Vector2 positionPoint(position.x, position.y);
    }
}

bool EditorCanvas::CanProcessInput() const
{
    return systemsManager->GetDragState() == EditorSystemsManager::DragScreen;
}

bool EditorCanvas::OnInput(UIEvent* currentInput)
{
    Vector2 delta(currentInput->point - lastMousePos);
    lastMousePos = currentInput->point;

    SetPosition(position + delta);
}

void EditorCanvas::UpdateDragScreenState()
{
    bool inDragScreenState = isMouseMidButtonPressed || (isMouseLeftButtonPressed && isSpacePressed);
    EditorSystemsManager::eDragState dragState_ = inDragScreenState ? EditorSystemsManager::DragScreen : EditorSystemsManager::DragControls;

    if (dragState_ == dragState)
    {
        return;
    }
    dragState = dragState_;
    systemsManager->dragStateChanged.Emit(dragState);
}