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

#ifndef __DAVAENGINE_DIRECTION_BASED_NAVIGATION_ALGORITHM_H__
#define __DAVAENGINE_DIRECTION_BASED_NAVIGATION_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"
#include "UI/Focus/UINavigationComponent.h"

namespace DAVA
{
class UIControl;
class UIList;
class UIEvent;

class DirectionBasedNavigationAlgorithm
{
public:
    DirectionBasedNavigationAlgorithm(UIControl* root);
    ~DirectionBasedNavigationAlgorithm();

    UIControl* GetNextControl(UIControl* focusedControl, UINavigationComponent::Direction dir);

private:
    UIControl* FindFirstControl(UIControl* control) const;
    UIControl* FindNextControl(UIControl* focusedControl, UINavigationComponent::Direction dir) const;
    UIControl* FindNextSpecifiedControl(UIControl* focusedControl, UINavigationComponent::Direction dir) const;
    UIControl* FindNearestControl(UIControl* focusedControl, UIControl* control, UINavigationComponent::Direction dir) const;
    Vector2 CalcNearestPos(const Vector2& pos, UIControl* testControl, UINavigationComponent::Direction dir) const;

    bool CanNavigateToControl(UIControl* focusedControl, UIControl* control, UINavigationComponent::Direction dir) const;
    UIControl* FindFirstControlImpl(UIControl* control, UIControl* candidate) const;

    RefPtr<UIControl> root;
};
}


#endif //__DAVAENGINE_DIRECTION_BASED_NAVIGATION_ALGORITHM_H__
