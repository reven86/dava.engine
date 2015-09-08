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


#include "ScrollAreaController.h"
#include "UI/UIScreenManager.h"

using namespace DAVA;

ScrollAreaController::ScrollAreaController(UIControl *root_, UIControl *scalableContent_, QObject *parent)
    : QObject(parent)
    , backgroundControl(new UIControl)
    , rootControl(root_)
    , scalableContent(scalableContent_)
{
    backgroundControl->AddControl(rootControl);
    rootControl->SetPosition(Vector2(Margin, Margin));

    ScopedPtr<UIScreen> davaUIScreen(new UIScreen());
    davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    davaUIScreen->GetBackground()->SetColor(Color(0.3f, 0.3f, 0.3f, 1.0f));
    UIScreenManager::Instance()->RegisterScreen(0, davaUIScreen);
    UIScreenManager::Instance()->SetFirst(0);
    UIScreenManager::Instance()->GetScreen()->AddControl(backgroundControl);
}

QSize ScrollAreaController::GetCanvasSize() const
{
    return canvasSize;
}

QSize ScrollAreaController::GetViewSize() const
{
    return viewSize;
}

int ScrollAreaController::GetScale() const
{
    return scale;
}

QPoint ScrollAreaController::GetPosition() const
{
    return position;
}

void ScrollAreaController::UpdateCanvasContentSize()
{
    const auto childs = scalableContent->GetChildren();
    UIControl* canvas = !childs.empty() ? childs.front() : nullptr;
    Vector2 contentSize;
    if (nullptr != canvas)
    {
        contentSize = canvas->GetGeometricData().GetAABBox().GetSize();
    }
    Vector2 marginsSize(Margin * 2, Margin * 2);
    Vector2 tmpSize = contentSize + marginsSize;
    backgroundControl->SetSize(tmpSize);
    canvasSize = QSize(tmpSize.dx, tmpSize.dy);
    UpdatePosition();
    emit CanvasSizeChanged(canvasSize);
}

void ScrollAreaController::SetViewSize(const QSize& viewSize_)
{
    if (viewSize_ != viewSize)
    {
        viewSize = viewSize_;
        auto newSize = Vector2(viewSize_.width(), viewSize_.height());
        UIScreenManager::Instance()->GetScreen()->SetSize(newSize);
        UpdatePosition();
        emit ViewSizeChanged(viewSize_);
    }
}

void ScrollAreaController::SetScale(int scale_)
{
    if (scale_ != scale)
    {
        scale = scale_;
        double uiScale = scale / 100.0f;
        Vector2 newScale(uiScale, uiScale);
        scalableContent->SetScale(newScale);
        UpdateCanvasContentSize();
        emit ScaleChanged(scale_);
    }
}

void ScrollAreaController::SetPosition(const QPoint &position_)
{
    if (position_ != position)
    {
        position = position_;
        auto newPos = Vector2(position.x(), position.y());
        backgroundControl->SetPosition(newPos);
        emit PositionChanged(position_);
    }
}

void ScrollAreaController::UpdatePosition()
{
    QPoint position(0, 0);
    if (viewSize.width() > canvasSize.width())
    {
        position.setX((viewSize.width() - canvasSize.width()) / 2.0f);
    }
    if (viewSize.height() > canvasSize.height())
    {
        position.setY((viewSize.height() - canvasSize.height()) / 2.0f);
    }
    SetPosition(position);
}
