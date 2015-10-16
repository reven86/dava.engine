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


#include "UIFlowLayoutHintComponent.h"

#include "UI/UIControl.h"
#include "Math/Vector.h"

namespace DAVA
{
    
UIFlowLayoutHintComponent::UIFlowLayoutHintComponent()
{
}

UIFlowLayoutHintComponent::UIFlowLayoutHintComponent(const UIFlowLayoutHintComponent &src)
    : flags(src.flags)
{
}

UIFlowLayoutHintComponent::~UIFlowLayoutHintComponent()
{
    
}

UIFlowLayoutHintComponent* UIFlowLayoutHintComponent::Clone() const
{
    return new UIFlowLayoutHintComponent(*this);
}

bool UIFlowLayoutHintComponent::IsNewLineBeforeThis() const
{
    return flags.test(FLAG_NEW_LINE_BEFORE_THIS);
}

void UIFlowLayoutHintComponent::SetNewLineBeforeThis(bool flag)
{
    flags.set(FLAG_NEW_LINE_BEFORE_THIS, flag);
    SetLayoutDirty();
}

bool UIFlowLayoutHintComponent::IsNewLineAfterThis() const
{
    return flags.test(FLAG_NEW_LINE_AFTER_THIS);
}

void UIFlowLayoutHintComponent::SetNewLineAfterThis(bool flag)
{
    flags.set(FLAG_NEW_LINE_AFTER_THIS, flag);
    SetLayoutDirty();
}

void UIFlowLayoutHintComponent::SetLayoutDirty()
{
    if (GetControl() != nullptr)
    {
        GetControl()->SetLayoutDirty();
    }
}
    
}
