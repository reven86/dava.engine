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

#ifndef __DAVAENGINE_UI_ANCHOR_HINT_COMPONENT_H__
#define __DAVAENGINE_UI_ANCHOR_HINT_COMPONENT_H__

#include "UI/Components/UIComponent.h"

namespace DAVA
{
    class UIAnchorHintComponent : public UIComponent
    {
    public:
        IMPLEMENT_COMPONENT_TYPE(ANCHOR_HINT_COMPONENT);
        
        UIAnchorHintComponent();
        UIAnchorHintComponent(const UIAnchorHintComponent &src);
        
    protected:
        virtual ~UIAnchorHintComponent();
        
    private:
        UIAnchorHintComponent &operator=(const UIAnchorHintComponent &) = delete;
        
    public:
        UIAnchorHintComponent* Clone() override;
        
        bool IsLeftAnchorEnabled() const;
        void SetLeftAnchorEnabled(bool enabled);
        
        float32 GetLeftAnchor() const;
        void SetLeftAnchor(float32 anchor);

        bool IsHCenterAnchorEnabled() const;
        void SetHCenterAnchorEnabled(bool enabled);
        
        float32 GetHCenterAnchor() const;
        void SetHCenterAnchor(float32 anchor);

        bool IsRightAnchorEnabled() const;
        void SetRightAnchorEnabled(bool enabled);
        
        float32 GetRightAnchor() const;
        void SetRightAnchor(float32 anchor);

        bool IsTopAnchorEnabled() const;
        void SetTopAnchorEnabled(bool enabled);
        
        float32 GetTopAnchor() const;
        void SetTopAnchor(float32 anchor);

        bool IsVCenterAnchorEnabled() const;
        void SetVCenterAnchorEnabled(bool enabled);
        
        float32 GetVCenterAnchor() const;
        void SetVCenterAnchor(float32 anchor);

        bool IsBottomAnchorEnabled() const;
        void SetBottomAnchorEnabled(bool enabled);
        
        float32 GetBottomAnchor() const;
        void SetBottomAnchor(float32 anchor);
        
        bool IsUseRtl() const;
        void SetUseRtl(bool use);

    private:
        static const int32 ANCHOR_LEFT = 0x01;
        static const int32 ANCHOR_HCENTER = 0x02;
        static const int32 ANCHOR_RIGHT = 0x04;
        static const int32 ANCHOR_TOP = 0x08;
        static const int32 ANCHOR_VCENTER = 0x10;
        static const int32 ANCHOR_BOTTOM = 0x20;
        
        int32 anchors = 0;
        float32 leftAnchor = 0.0f;
        float32 hCenterAnchor = 0.0f;
        float32 rightAnchor = 0.0f;
        float32 topAnchor = 0.0f;
        float32 vCenterAnchor = 0.0f;
        float32 bottomAnchor = 0.0f;
        bool useRtl = false;

    public:
        INTROSPECTION_EXTEND(UIAnchorHintComponent, UIComponent,
                             PROPERTY("leftAnchorEnabled", "Left Anchor Enabled", IsLeftAnchorEnabled, SetLeftAnchorEnabled, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("leftAnchor", "Left Anchor", GetLeftAnchor, SetLeftAnchor, I_SAVE | I_VIEW | I_EDIT)

                             PROPERTY("hCenterAnchorEnabled", "HCenter Anchor Enabled", IsHCenterAnchorEnabled, SetHCenterAnchorEnabled, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("hCenterAnchor", "HCenter Anchor", GetHCenterAnchor, SetHCenterAnchor, I_SAVE | I_VIEW | I_EDIT)

                             PROPERTY("rightAnchorEnabled", "Right Anchor Enabled", IsRightAnchorEnabled, SetRightAnchorEnabled, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("rightAnchor", "Right Anchor", GetRightAnchor, SetRightAnchor, I_SAVE | I_VIEW | I_EDIT)

                             PROPERTY("topAnchorEnabled", "Top Anchor Enabled", IsTopAnchorEnabled, SetTopAnchorEnabled, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("topAnchor", "Top Anchor", GetTopAnchor, SetTopAnchor, I_SAVE | I_VIEW | I_EDIT)

                             PROPERTY("vCenterAnchorEnabled", "VCenter Anchor Enabled", IsVCenterAnchorEnabled, SetVCenterAnchorEnabled, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("vCenterAnchor", "VCenter Anchor", GetVCenterAnchor, SetVCenterAnchor, I_SAVE | I_VIEW | I_EDIT)

                             PROPERTY("bottomAnchorEnabled", "Bottom Anchor Enabled", IsBottomAnchorEnabled, SetBottomAnchorEnabled, I_SAVE | I_VIEW | I_EDIT)
                             PROPERTY("bottomAnchor", "Bottom Anchor", GetBottomAnchor, SetBottomAnchor, I_SAVE | I_VIEW | I_EDIT)
                             
                             PROPERTY("useRtl", "Use Rtl Align", IsUseRtl, SetUseRtl, I_SAVE | I_VIEW | I_EDIT)
                             );
        
    };
    
}


#endif //__DAVAENGINE_UI_ANCHOR_HINT_COMPONENT_H__
