#include "ScrollAreaController.h"
#include "UI/UIScreenManager.h"

using namespace DAVA;

ScrollAreaController::ScrollAreaController(QObject* parent)
    : QObject(parent)
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

ScrollAreaController::~ScrollAreaController()
{
    UIScreenManager::Instance()->ResetScreen();
}

void ScrollAreaController::SetNestedControl(UIControl* arg)
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

void ScrollAreaController::SetMovableControl(UIControl* arg)
{
    if (arg != movableControl)
    {
        movableControl = arg;
        UpdatePosition();
    }
}

void ScrollAreaController::AdjustScale(float newScale, const QPointF& mousePos)
{
    newScale = fmax(minScale, newScale);
    newScale = fmin(maxScale, newScale); //crop scale to 800
    if (scale == newScale)
    {
        return;
    }
    QPoint oldPos = position;
    float oldScale = scale;
    scale = newScale;
    UpdateCanvasContentSize();
    emit ScaleChanged(scale);

    if (oldScale == 0 || viewSize.width() <= 0 || viewSize.height() <= 0)
    {
        SetPosition(QPoint(0, 0));
        return;
    }

    QPoint absPosition = oldPos / oldScale;
    QPointF deltaMousePos = mousePos * (1 - newScale / oldScale);
    QPoint newPosition(absPosition.x() * scale - deltaMousePos.x(), absPosition.y() * scale - deltaMousePos.y());

    newPosition.setX(qBound(0, newPosition.x(), (canvasSize - viewSize).width()));
    newPosition.setY(qBound(0, newPosition.y(), (canvasSize - viewSize).height()));
    SetPosition(newPosition);
}

QSize ScrollAreaController::GetCanvasSize() const
{
    return canvasSize;
}

QSize ScrollAreaController::GetViewSize() const
{
    return viewSize;
}

QPoint ScrollAreaController::GetPosition() const
{
    return position;
}

float ScrollAreaController::GetScale() const
{
    return scale;
}

float ScrollAreaController::GetMinScale() const
{
    return minScale;
}

float ScrollAreaController::GetMaxScale() const
{
    return maxScale;
}

QPoint ScrollAreaController::GetMinimumPos() const
{
    return QPoint(0, 0);
}

QPoint ScrollAreaController::GetMaximumPos() const
{
    QSize maxSize = canvasSize - viewSize;
    return QPoint(maxSize.width(), maxSize.height());
}

void ScrollAreaController::UpdateCanvasContentSize()
{
    Vector2 contentSize;
    if (nullptr != nestedControl)
    {
        const auto& gd = nestedControl->GetGeometricData();

        contentSize = gd.GetAABBox().GetSize() * scale;
    }
    Vector2 marginsSize(Margin * 2, Margin * 2);
    Vector2 tmpSize = contentSize + marginsSize;
    backgroundControl->SetSize(tmpSize);
    canvasSize = QSize(tmpSize.dx, tmpSize.dy);
    UpdatePosition();
    emit CanvasSizeChanged(canvasSize);
}

void ScrollAreaController::SetScale(float arg)
{
    if (scale != arg)
    {
        AdjustScale(arg, QPoint(viewSize.width() / 2, viewSize.height() / 2)); //like cursor at center of view
    }
}

void ScrollAreaController::SetViewSize(int32 width, int32 height)
{
    SetViewSize(QSize(width, height));
}

void ScrollAreaController::SetViewSize(const QSize& viewSize_)
{
    if (viewSize_ != viewSize)
    {
        viewSize = viewSize_;

        DAVA::VirtualCoordinatesSystem* vcs = DAVA::UIControlSystem::Instance()->vcs;

        vcs->UnregisterAllAvailableResourceSizes();
        vcs->SetVirtualScreenSize(viewSize.width(), viewSize.height());
        vcs->RegisterAvailableResourceSize(viewSize.width(), viewSize.height(), "Gfx");
        vcs->RegisterAvailableResourceSize(viewSize.width(), viewSize.height(), "Gfx2");

        auto newSize = Vector2(viewSize_.width(), viewSize_.height());
        UIScreenManager::Instance()->GetScreen()->SetSize(newSize);
        UpdatePosition();
        emit ViewSizeChanged(viewSize);
    }
}

void ScrollAreaController::SetPosition(const QPoint& position_)
{
    QPoint minPos = GetMinimumPos();
    QPoint maxPos = GetMaximumPos();
    QPoint fixedPos(qBound(minPos.x(), position_.x(), maxPos.x()),
                    qBound(minPos.y(), position_.y(), maxPos.y()));
    if (fixedPos != position)
    {
        position = fixedPos;
        UpdatePosition();
        emit PositionChanged(position);
    }
}

void ScrollAreaController::UpdatePosition()
{
    if (nullptr != movableControl)
    {
        QSize offset = (canvasSize - viewSize) / 2;

        if (offset.width() > 0)
        {
            offset.setWidth(position.x());
        }
        if (offset.height() > 0)
        {
            offset.setHeight(position.y());
        }
        offset -= QSize(Margin, Margin);
        Vector2 position(-offset.width(), -offset.height());
        movableControl->SetPosition(position);

        QPoint positionPoint(static_cast<int>(position.x), static_cast<int>(position.y));
        NestedControlPositionChanged(positionPoint);
    }
}
