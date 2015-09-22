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

#ifndef __DAVAENGINE_SIZE_MEASURING_ALGORITHM_H__
#define __DAVAENGINE_SIZE_MEASURING_ALGORITHM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

#include "ControlLayoutData.h"

namespace DAVA
{
class UISizePolicyComponent;
class UIFlowLayoutComponent;
class UILinearLayoutComponent;
    
class SizeMeasuringAlgorithm
{
public:
    SizeMeasuringAlgorithm(Vector<ControlLayoutData> &layoutData_);
    ~SizeMeasuringAlgorithm();
    
    void Apply(ControlLayoutData &data, Vector2::eAxis axis);
    
private:
    void ProcessIgnoreSizePolicy(ControlLayoutData &data, Vector2::eAxis axis);
    void ProcessFixedSizePolicy(ControlLayoutData &data, Vector2::eAxis axis);
    void ProcessPercentOfChildrenSumPolicy(ControlLayoutData &data, Vector2::eAxis axis);
    void ProcessPercentOfMaxChildPolicy(ControlLayoutData &data, Vector2::eAxis axis);
    void ProcessPercentOfFirstChildPolicy(ControlLayoutData &data, Vector2::eAxis axis);
    void ProcessPercentOfLastChildPolicy(ControlLayoutData &data, Vector2::eAxis axis);
    void ProcessPercentOfContentPolicy(ControlLayoutData &data, Vector2::eAxis axis);
    void ProcessPercentOfParentPolicy(ControlLayoutData &data, Vector2::eAxis axis);

    void ApplySize(ControlLayoutData &data, float32 value, Vector2::eAxis axis);
    float32 GetSize(ControlLayoutData &data, Vector2::eAxis axis);
    float32 GetLayoutPadding(Vector2::eAxis axis);
    float32 ClampValue(float32 value, Vector2::eAxis axis);

private:
    Vector<ControlLayoutData> &layoutData;
    
    UISizePolicyComponent *sizePolicy = nullptr;
    UILinearLayoutComponent *linearLayout = nullptr;
    UIFlowLayoutComponent *flowLayout = nullptr;
    
    bool skipInvisible = false;
};

}


#endif //__DAVAENGINE_SIZE_MEASURING_ALGORITHM_H__
