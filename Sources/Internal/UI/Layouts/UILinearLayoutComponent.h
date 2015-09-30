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

#ifndef __DAVAENGINE_UI_LINEAR_LAYOUT_COMPONENT_H__
#define __DAVAENGINE_UI_LINEAR_LAYOUT_COMPONENT_H__

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UILinearLayoutComponent : public UIComponent
{
public:
    enum eOrientation {
        HORIZONTAL = 0,
        VERTICAL = 1
    };
    
public:
    IMPLEMENT_COMPONENT_TYPE(LINEAR_LAYOUT_COMPONENT);
    
    UILinearLayoutComponent();
    UILinearLayoutComponent(const UILinearLayoutComponent &src);
    
protected:
    virtual ~UILinearLayoutComponent();
    
private:
    UILinearLayoutComponent &operator=(const UILinearLayoutComponent &) = delete;
    
public:
    virtual UILinearLayoutComponent* Clone() const override;
    
    bool IsEnabled() const;
    void SetEnabled(bool enabled);
    
    eOrientation GetOrientation() const;
    void SetOrientation(eOrientation orientation);
    
    Vector2::eAxis GetAxis() const;
    
    float32 GetPadding() const;
    void SetPadding(float32 padding);
    
    float32 GetSpacing() const;
    void SetSpacing(float32 spacing);
    
    bool IsDynamicPadding() const;
    void SetDynamicPadding(bool dynamic);
    
    bool IsDynamicSpacing() const;
    void SetDynamicSpacing(bool dynamic);
    bool IsUseRtl() const;
    void SetUseRtl(bool use);
    
    bool IsSkipInvisibleControls() const;
    void SetSkipInvisibleControls(bool skip);
    
private:
    int32 GetOrientationAsInt() const;
    void SetOrientationFromInt(int32 type);

    void SetLayoutDirty();
    
private:
    enum eFlags
    {
        FLAG_ENABLED,
        FLAG_ORIENTATION_VERTICAL,
        FLAG_ORIENTATION_INVERSE,
        FLAG_DYNAMIC_PADDING,
        FLAG_DYNAMIC_SPACING,
        FLAG_RTL,
        FLAG_SKIP_INVISIBLE_CONTROLS,
        FLAG_COUNT
    };
    
    Bitset<eFlags::FLAG_COUNT> flags;
    float32 padding = 0.0f;
    float32 spacing = 0.0f;
    
public:
    INTROSPECTION_EXTEND(UILinearLayoutComponent, UIComponent,
                         PROPERTY("enabled", "Enabled", IsEnabled, SetEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("orientation", InspDesc("Orientation", GlobalEnumMap<eOrientation>::Instance()), GetOrientationAsInt, SetOrientationFromInt, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("padding", "Padding", GetPadding, SetPadding, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("dynamicPadding", "Dynamic Padding", IsDynamicPadding, SetDynamicPadding, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("spacing", "Spacing", GetSpacing, SetSpacing, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("dynamicSpacing", "Dynamic Spacing", IsDynamicSpacing, SetDynamicSpacing, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("skipInvisible", "Skip Invisible Controls", IsSkipInvisibleControls, SetSkipInvisibleControls, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("useRtl", "Use Rtl Align", IsUseRtl, SetUseRtl, I_SAVE | I_VIEW | I_EDIT)
                         );
    
};

}


#endif //__DAVAENGINE_UI_LINEAR_LAYOUT_COMPONENT_H__
