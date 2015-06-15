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

#include "UI/UIStyleSheetSystem.h"
#include "UI/UIStyleSheet.h"
#include "UI/UIStyleSheetCascade.h"
#include "UI/UIControl.h"
#include "UI/UIControlPackageContext.h"
#include "UI/Components/UIComponent.h"
#include "Platform/SystemTimer.h"
#include "Platform/Thread.h"

namespace DAVA
{
    UIStyleSheetSystem::UIStyleSheetSystem()
    {

    }

    UIStyleSheetSystem::~UIStyleSheetSystem()
    {

    }

    void UIStyleSheetSystem::MarkControlForUpdate(UIControl* control)
    {
        controlsToUpdate.push_back(SafeRetain(control));
    }
    
    void UIStyleSheetSystem::Process()
    {
        if (!controlsToUpdate.empty())
        {
            uint64 start = SystemTimer::Instance()->AbsoluteMS();

            std::sort(controlsToUpdate.begin(), controlsToUpdate.end());
            auto endIter = std::unique(controlsToUpdate.begin(), controlsToUpdate.end());

            for (auto controlIter = controlsToUpdate.begin(); controlIter != endIter; ++controlIter)
            {
                ProcessControl(*controlIter);
                SafeRelease(*controlIter);
            }

            controlsToUpdate.clear();

            uint64 end = SystemTimer::Instance()->AbsoluteMS();

            DAVA::Logger::Debug("%s took %llu", __FUNCTION__, end - start);
        }
    }

    void UIStyleSheetSystem::ProcessControl(UIControl* control)
    {
        static UIStyleSheetCascade cascade;

        cascade.Clear();

        UIControlPackageContext* packageContext = control->GetPackageContext();

        if (packageContext)
        {
            const auto& styleSheets = packageContext->GetSortedStyleSheets();
            for (const UIStyleSheet* styleSheet : styleSheets)
            {
                if (StyleSheetMatchesControl(styleSheet, control))
                {
                    cascade.AddStyleSheet(styleSheet);
                }
            }
        }

        SetupControlFromCascade(control, cascade);
    }

    bool UIStyleSheetSystem::StyleSheetMatchesControl(const UIStyleSheet* styleSheet, UIControl* control)
    {
        UIControl* currentControl = control;

        auto endIter = styleSheet->GetSelectorChain().rend();
        for (auto selectorIter = styleSheet->GetSelectorChain().rbegin(); selectorIter != endIter; ++selectorIter)
        {
            if (!currentControl || !SelectorMatchesControl(*selectorIter, currentControl))
                return false;

            currentControl = currentControl->GetParent();
        }

        return true;
    }

    bool UIStyleSheetSystem::SelectorMatchesControl(const UIStyleSheetSelector& selector, UIControl* control)
    {
        if ((selector.name.IsValid() && selector.name != control->GetFastName())
            || (!selector.controlClassName.empty() && selector.controlClassName != control->GetClassName()))
            return false;

        for (const FastName& clazz : selector.classes)
        {
            if (!control->HasClass(clazz))
                return false;
        }

        return true;
    }
    
    void UIStyleSheetSystem::SetupControlFromCascade(UIControl* control, const UIStyleSheetCascade& cascade)
    {
        UIStyleSheetPropertySet propertiesToSet = cascade.GetPropertySet() &(~control->GetLocalPropertySet());

        for (uint32 propertyIndex = 0; propertyIndex < STYLE_SHEET_PROPERTY_COUNT; ++propertyIndex)
        {
            if (propertiesToSet.test(propertyIndex))
            {
                const UIStyleSheetPropertyDescriptor& descr = GetStyleSheetPropertyByIndex(propertyIndex);
                const VariantType* value = cascade.GetProperty(propertyIndex);
                
                switch (descr.owner)
                {
                case ePropertyOwner::CONTROL:
                {
                    const InspInfo* typeInfo = control->GetTypeInfo();
                    do
                    {
                        if (typeInfo == descr.typeInfo)
                        {
                            descr.inspMember->SetValue(control, *value);
                            break;
                        }
                        typeInfo = typeInfo->BaseInfo();
                    } while (typeInfo);

                    break;
                }
                case ePropertyOwner::BACKGROUND:
                    if (control->GetBackgroundComponentsCount() > 0)
                        descr.inspMember->SetValue(control->GetBackgroundComponent(0), *value);
                    break;
                case ePropertyOwner::COMPONENT:
                    for (const auto& componentInfo : descr.targetComponents)
                    {
                        if (UIComponent* component = control->GetComponent(componentInfo.first))
                            componentInfo.second->SetValue(component, *value);
                    }
                    break;
                }
            }
        }
    }
}
