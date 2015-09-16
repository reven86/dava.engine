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

#ifndef __DAVAENGINE_UI_LAYOUT_SYSTEM_H__
#define __DAVAENGINE_UI_LAYOUT_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/Vector.h"

namespace DAVA
{
class UIControl;
class UILinearLayoutComponent;
class UIFlowLayoutComponent;
class UISizePolicyComponent;

class UILayoutSystem
{
public:
    UILayoutSystem();
    virtual ~UILayoutSystem();
    
public:
    bool IsRtl() const;
    void SetRtl(bool rtl);

    void ApplyLayout(UIControl *control);
    UIControl *FindControl(UIControl *control) const;
    
private:
    static const int32 FLAG_SIZE_CHANGED = 0x01;
    static const int32 FLAG_POSITION_CHANGED = 0x02;
    struct ControlDescr
    {
        UIControl *control;
        int32 flags;
        int32 firstChild;
        int32 lastChild;
        Vector2 size;
        Vector2 position;
        
        bool HasChildren() const {
            return lastChild >= firstChild;
        }
    };

private:
    void CollectControls(UIControl *control);
    void CollectControlChildren(UIControl *control, int32 parentIndex);
    
    void MeasureControl(ControlDescr &descr, Vector2::eAxis axis);
    void ApplyLinearLayout(ControlDescr &descr, UILinearLayoutComponent *linearLayoutComponent, Vector2::eAxis axis);
    void ApplyAnchorLayout(ControlDescr &descr, Vector2::eAxis axis, bool onlyForIgnoredControls);

    bool HaveToSkipControl(UIControl *control, bool skipInvisible) const;
    
private:
    bool isRtl = false;
    Vector<ControlDescr> controls;
    int32 indexOfSizeProperty;
};


}


#endif //__DAVAENGINE_UI_LAYOUT_SYSTEM_H__
