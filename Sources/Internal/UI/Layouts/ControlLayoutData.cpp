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

#include "ControlLayoutData.h"

#include "UI/UIControl.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{

ControlLayoutData::ControlLayoutData(UIControl *control_) : control(control_)
{
    position = control->GetPosition() - control->GetPivotPoint();
    size = control->GetSize();
}

void ControlLayoutData::ApplyLayoutToControl(int32 indexOfSizeProperty)
{
    if (HasFlag(FLAG_POSITION_CHANGED))
    {
        control->SetPosition(position + control->GetPivotPoint());
    }
    
    if (HasFlag(FLAG_SIZE_CHANGED))
    {
        control->SetSize(size);
        control->SetPropertyLocalFlag(indexOfSizeProperty, true);
        control->OnSizeChanged();
    }
    
    control->ResetLayoutDirty();
}

UIControl *ControlLayoutData::GetControl() const
{
    return control;
}

bool ControlLayoutData::HasFlag(eFlag flag) const
{
    return (flags & flag) == flag;
}

void ControlLayoutData::SetFlag(eFlag flag)
{
    flags |= flag;
}

int32 ControlLayoutData::GetFirstChildIndex() const
{
    return firstChild;
}

void ControlLayoutData::SetFirstChildIndex(int32 index)
{
    firstChild = index;
}

int32 ControlLayoutData::GetLastChildIndex() const
{
    return lastChild;
}

void ControlLayoutData::SetLastChildIndex(int32 index)
{
    lastChild = index;
}

bool ControlLayoutData::HasChildren() const
{
    return lastChild >= firstChild;
}

float32 ControlLayoutData::GetSize(Vector2::eAxis axis) const
{
    return size.data[axis];
}

void ControlLayoutData::SetSize(Vector2::eAxis axis, float32 value)
{
    size.data[axis] = value;
    flags |= FLAG_SIZE_CHANGED;
}

void ControlLayoutData::SetSizeWithoutChangeFlag(Vector2::eAxis axis, float32 value)
{
    size.data[axis] = value;
}

float32 ControlLayoutData::GetPosition(Vector2::eAxis axis) const
{
    return position.data[axis];
}

void ControlLayoutData::SetPosition(Vector2::eAxis axis, float32 value)
{
    position.data[axis] = value;
    flags |= FLAG_POSITION_CHANGED;
}

float32 ControlLayoutData::GetX() const
{
    return position.x;
}

float32 ControlLayoutData::GetY() const
{
    return position.y;
}

float32 ControlLayoutData::GetWidth() const
{
    return size.dx;
}

float32 ControlLayoutData::GetHeight() const
{
    return size.dy;
}

bool ControlLayoutData::HaveToSkipControl(bool skipInvisible) const
{
    return (skipInvisible && !control->GetVisible()) || control->GetComponentCount(UIComponent::IGNORE_LAYOUT_COMPONENT) > 0;
}
}
