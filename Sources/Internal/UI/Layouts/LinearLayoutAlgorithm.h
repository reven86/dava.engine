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

#ifndef __DAVAENGINE_LINEAR_LAYOUT_ALGORITHM_H__
#define __DAVAENGINE_LINEAR_LAYOUT_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "ControlLayoutData.h"

namespace DAVA
{
    
class UIControl;
class UILinearLayoutComponent;
class UISizePolicyComponent;

class LinearLayoutAlgorithm
{
public:
    LinearLayoutAlgorithm(Vector<ControlLayoutData> &layoutData_, bool isRtl_);
    ~LinearLayoutAlgorithm();
    
    void Apply(ControlLayoutData &data, Vector2::eAxis axis);
    
private:
    void InitializeParams(ControlLayoutData &data, const UILinearLayoutComponent *layout, Vector2::eAxis axis);
    void CalculateDependentOnParentSizes(ControlLayoutData &data, Vector2::eAxis axis);
    bool CalculateChildDependentOnParentSize(ControlLayoutData &data, Vector2::eAxis axis);
    void CalculateDynamicPaddingAndSpaces(ControlLayoutData &data, const UILinearLayoutComponent *layout, Vector2::eAxis axis);
    void PlaceChildren(ControlLayoutData &data, Vector2::eAxis axis);

private:
    Vector<ControlLayoutData> &layoutData;
    bool isRtl;
    
    bool inverse = false;
    bool skipInvisible = true;
    
    float32 fixedSize = 0.0f;
    float32 totalPercent = 0.0f;

    float32 contentSize = 0.0f;
    float32 restSize = 0.0f;

    int32 childrenCount = 0;
    int32 spacesCount = 0;

    float32 padding = 0.0f;
    float32 spacing = 0.0f;
};

}


#endif //__DAVAENGINE_LINEAR_LAYOUT_ALGORITHM_H__
