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

    void UIStyleSheetSystem::Process()
    {
        if (!controlsToUpdate.empty())
        {

            std::sort(controlsToUpdate.begin(), controlsToUpdate.end());
            auto endIter = std::unique(controlsToUpdate.begin(), controlsToUpdate.end());

            uint64 start = SystemTimer::Instance()->AbsoluteMS();

            for (auto controlIter = controlsToUpdate.begin(); controlIter != endIter; ++controlIter)
            {
                ProcessControl(*controlIter);
                SafeRelease(*controlIter);
            }


            uint64 end = SystemTimer::Instance()->AbsoluteMS();

            DAVA::Logger::Debug("%s (%i) took %llu", __FUNCTION__, std::distance(controlsToUpdate.begin(), endIter), end - start);

            controlsToUpdate.clear();
        }
    }

    void UIStyleSheetSystem::MarkControlForUpdate(UIControl* control)
    {
        UIControlPackageContext* packageContext = control->GetPackageContext();
        if (packageContext)
        {
            MarkControlForUpdate(control, packageContext->GetMaxStyleSheetSelectorDepth());
        }
    }

    void UIStyleSheetSystem::MarkControlForUpdate(UIControl* control, int32 depth)
    {
        controlsToUpdate.push_back(SafeRetain(control));
        if (depth > 1)
        {
            for (UIControl* child : control->GetChildren())
                MarkControlForUpdate(child, depth - 1);
        }
    }

    void UIStyleSheetSystem::ProcessControl(UIControl* control)
    {
        UIControlPackageContext* packageContext = control->GetPackageContext();

        if (packageContext)
        {
            UIStyleSheetPropertySet appliedProperties;
            const UIStyleSheetPropertySet& localControlProperties = control->GetLocalPropertySet();

            const auto& styleSheets = packageContext->GetSortedStyleSheets();
            for (const UIStyleSheet* styleSheet : styleSheets)
            {
                if (StyleSheetMatchesControl(styleSheet, control))
                {
                    const auto& propertyTable = styleSheet->GetPropertyTable()->GetProperties();
                    for (const auto& iter : propertyTable)
                    {
                        if (!appliedProperties.test(iter.first) && !localControlProperties.test(iter.first))
                        {
                            appliedProperties.set(iter.first);

                            SetupPropertyFromVariantType(control, iter.first, iter.second);
                        }
                    }
                }
            }
            control->SetStyledPropertySet(appliedProperties);
        }
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

    void UIStyleSheetSystem::SetupPropertyFromVariantType(UIControl* control, int32 propertyIndex, const VariantType& value)
    {
        const UIStyleSheetPropertyDescriptor& descr = GetStyleSheetPropertyByIndex(propertyIndex);

        switch (descr.owner)
        {
            case ePropertyOwner::CONTROL:
            {
                const InspInfo* typeInfo = control->GetTypeInfo();
                do
                {
                    if (typeInfo == descr.typeInfo)
                    {
                        descr.inspMember->SetValue(control, value);
                        break;
                    }
                    typeInfo = typeInfo->BaseInfo();
                } while (typeInfo);

                break;
            }
            case ePropertyOwner::BACKGROUND:
                if (control->GetBackgroundComponentsCount() > 0)
                    descr.inspMember->SetValue(control->GetBackgroundComponent(0), value);
                break;
            case ePropertyOwner::COMPONENT:
                for (const auto& componentInfo : descr.targetComponents)
                {
                    if (UIComponent* component = control->GetComponent(componentInfo.first))
                        componentInfo.second->SetValue(component, value);
                }
                break;
            default:
                DVASSERT(false);
                break;
        }
    }
}
