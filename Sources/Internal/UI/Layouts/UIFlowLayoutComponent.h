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

#ifndef __DAVAENGINE_UI_FLOW_LAYOUT_COMPONENT_H__
#define __DAVAENGINE_UI_FLOW_LAYOUT_COMPONENT_H__

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIFlowLayoutComponent : public UIComponent
{
public:
    enum eOrientation
    {
        ORIENTATION_LEFT_TO_RIGHT,
        ORIENTATION_RIGHT_TO_LEFT
    };
    
public:
    IMPLEMENT_UI_COMPONENT_TYPE(FLOW_LAYOUT_COMPONENT);
    
    UIFlowLayoutComponent();
    UIFlowLayoutComponent(const UIFlowLayoutComponent &src);
    
protected:
    virtual ~UIFlowLayoutComponent();
    
private:
    UIFlowLayoutComponent &operator=(const UIFlowLayoutComponent &) = delete;
    
public:
    virtual UIFlowLayoutComponent* Clone() const override;
    
    bool IsEnabled() const;
    void SetEnabled(bool enabled);
    
    eOrientation GetOrientation() const;
    void SetOrientation(eOrientation orientation);
    
    float32 GetHorizontalPadding() const;
    void SetHorizontalPadding(float32 padding);
    
    float32 GetHorizontalSpacing() const;
    void SetHorizontalSpacing(float32 spacing);
    
    bool IsDynamicHorizontalPadding() const;
    void SetDynamicHorizontalPadding(bool dynamic);
    
    bool IsDynamicHorizontalInLinePadding() const;
    void SetDynamicHorizontalInLinePadding(bool dynamic);
    
    bool IsDynamicHorizontalSpacing() const;
    void SetDynamicHorizontalSpacing(bool dynamic);

    float32 GetVerticalPadding() const;
    void SetVerticalPadding(float32 padding);
    
    float32 GetVerticalSpacing() const;
    void SetVerticalSpacing(float32 spacing);
    
    bool IsDynamicVerticalPadding() const;
    void SetDynamicVerticalPadding(bool dynamic);
    
    bool IsDynamicVerticalSpacing() const;
    void SetDynamicVerticalSpacing(bool dynamic);

    float32 GetPaddingByAxis(int32 axis);
    float32 GetSpacingByAxis(int32 axis);
    
    bool IsUseRtl() const;
    void SetUseRtl(bool use);
    
    bool IsSkipInvisibleControls() const;
    void SetSkipInvisibleControls(bool skip);
    
private:
    int32 GetOrientationAsInt() const;
    void SetOrientationFromInt(int32 orientation);
    
    void SetLayoutDirty();

private:
    enum eFlags
    {
        FLAG_ENABLED,
        FLAG_DYNAMIC_HORIZONTAL_PADDING,
        FLAG_DYNAMIC_HORIZONTAL_IN_LINE_PADDING,
        FLAG_DYNAMIC_HORIZONTAL_SPACING,
        FLAG_DYNAMIC_VERTICAL_PADDING,
        FLAG_DYNAMIC_VERTICAL_SPACING,
        FLAG_USE_RTL,
        FLAG_SKIP_INVISIBLE_CONTROLS,
        FLAG_IS_RIGHT_TO_LEFT,
        FLAG_COUNT
    };
    
    Array<float32, Vector2::AXIS_COUNT> padding;
    Array<float32, Vector2::AXIS_COUNT> spacing;
    
    Bitset<eFlags::FLAG_COUNT> flags;
    
public:
    INTROSPECTION_EXTEND(UIFlowLayoutComponent, UIComponent,
                         PROPERTY("enabled", "Enabled", IsEnabled, SetEnabled, I_SAVE | I_VIEW | I_EDIT)
                         
                         PROPERTY("orientation", InspDesc("Orientation", GlobalEnumMap<eOrientation>::Instance()), GetOrientationAsInt, SetOrientationFromInt, I_SAVE | I_VIEW | I_EDIT)
                         
                         PROPERTY("hPadding", "Horizontal Padding", GetHorizontalPadding, SetHorizontalPadding, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("hDynamicPadding", "Dynamic Horizontal Padding", IsDynamicHorizontalPadding, SetDynamicHorizontalPadding, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("hDynamicInLinePadding", "Dynamic In Line Padding", IsDynamicHorizontalInLinePadding, SetDynamicHorizontalInLinePadding, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("hSpacing", "Horizontal Spacing", GetHorizontalSpacing, SetHorizontalSpacing, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("hDynamicSpacing", "Dynamic Horizontal Spacing", IsDynamicHorizontalSpacing, SetDynamicHorizontalSpacing, I_SAVE | I_VIEW | I_EDIT)
                         
                         PROPERTY("vPadding", "Vertical Padding", GetVerticalPadding, SetVerticalPadding, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("vDynamicPadding", "Dynamic Vertical Padding", IsDynamicVerticalPadding, SetDynamicVerticalPadding, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("vSpacing", "Vertical Spacing", GetVerticalSpacing, SetVerticalSpacing, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("vDynamicSpacing", "Dynamic Vertical Spacing", IsDynamicVerticalSpacing, SetDynamicVerticalSpacing, I_SAVE | I_VIEW | I_EDIT)

                         PROPERTY("skipInvisible", "Skip Invisible Controls", IsSkipInvisibleControls, SetSkipInvisibleControls, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("useRtl", "Use Rtl Align", IsUseRtl, SetUseRtl, I_SAVE | I_VIEW | I_EDIT)
                         );
    
};

}


#endif //__DAVAENGINE_UI_FLOW_LAYOUT_COMPONENT_H__
