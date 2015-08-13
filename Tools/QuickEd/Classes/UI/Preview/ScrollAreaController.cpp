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
#include "UI/UIControl.h"
#include "UI/UIScreenManager.h"

using namespace DAVA;

ScrollAreaController::ScrollAreaController(UIControl *root, QObject *parent)
    : QObject(parent)
    , rootControl(root)
{
    
}

QSize ScrollAreaController::GetSize() const
{
    return size;
}

int ScrollAreaController::GetScale() const
{
    return scale;
}

QPoint ScrollAreaController::GetPosition() const
{
    return position;
}

void ScrollAreaController::SetSize(const QSize &size_)
{
    if (size_ != size)
    {
        size = size_;
        auto newSize = Vector2(size.width(), size.height());
        UIScreenManager::Instance()->GetScreen()->SetSize(newSize);
        emit SizeChanged(size_);
    }
}

void ScrollAreaController::SetScale(int scale_)
{
    if (scale_ != scale)
    {
        scale = scale_;
        Vector2 newScale(static_cast<float>(scale) / 100.0f, static_cast<float>(scale) / 100.0f);
        rootControl->SetScale(newScale);
        emit ScaleChanged(scale_);
    }
}

void ScrollAreaController::SetPosition(const QPoint &position_)
{
    if (position_ != position)
    {
        position = position_;
        auto newPos = Vector2(position.x(), position.y());
        rootControl->SetPosition(newPos);
        emit PositionChanged(position_);
    }
}
