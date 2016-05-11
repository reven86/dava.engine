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

#include "UINavigationComponent.h"

namespace DAVA
{
UINavigationComponent::UINavigationComponent()
{
}

UINavigationComponent::UINavigationComponent(const UINavigationComponent& src)
{
    for (int i = 0; i < DIRECTION_COUNT; i++)
    {
        nextFocusPath[i] = src.nextFocusPath[i];
    }
}

UINavigationComponent::~UINavigationComponent()
{
}

UINavigationComponent* UINavigationComponent::Clone() const
{
    return new UINavigationComponent(*this);
}

const String& UINavigationComponent::GetNextFocusLeft() const
{
    return nextFocusPath[LEFT];
}

void UINavigationComponent::SetNextFocusLeft(const String& val)
{
    nextFocusPath[LEFT] = val;
}

const String& UINavigationComponent::GetNextFocusRight() const
{
    return nextFocusPath[RIGHT];
}

void UINavigationComponent::SetNextFocusRight(const String& val)
{
    nextFocusPath[RIGHT] = val;
}

const String& UINavigationComponent::GetNextFocusUp() const
{
    return nextFocusPath[UP];
}

void UINavigationComponent::SetNextFocusUp(const String& val)
{
    nextFocusPath[UP] = val;
}

const String& UINavigationComponent::GetNextFocusDown() const
{
    return nextFocusPath[DOWN];
}

void UINavigationComponent::SetNextFocusDown(const String& val)
{
    nextFocusPath[DOWN] = val;
}

const String& UINavigationComponent::GetNextControlPathInDirection(Direction dir)
{
    DVASSERT(0 <= dir && dir < DIRECTION_COUNT);
    return nextFocusPath[dir];
}
}
