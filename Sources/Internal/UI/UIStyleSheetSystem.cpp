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
#include "UI/Components/UIComponent.h"

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
        controlsToUpdate.push_back(control);
        for (UIControl* child : control->GetChildren())
        {
            MarkControlForUpdate(child);
        }
    }

    void UIStyleSheetSystem::RegisterStyleSheet(UIStyleSheet* styleSheet)
    {
        styleSheets.push_back(SafeRetain(styleSheet));
        SortStyleSheets();
    }

    void UIStyleSheetSystem::UnregisterStyleSheet(UIStyleSheet* styleSheet)
    {
        auto iter = std::find(styleSheets.begin(), styleSheets.end(), styleSheet);
        if (iter != styleSheets.end())
        {
            SafeRelease(styleSheet);
            styleSheets.erase(iter);
        }
    }

    void UIStyleSheetSystem::ProcessUpdates()
    {
        static UIStyleSheetCascade cascade; // TODO
        cascade.Clear();

        std::sort(controlsToUpdate.begin(), controlsToUpdate.end());
        std::unique(controlsToUpdate.begin(), controlsToUpdate.end());

        for (UIControl* control : controlsToUpdate)
        {
            for (UIStyleSheet* styleSheet : styleSheets)
            {
                if (StyleSheetMatchesControl(styleSheet, control))
                {
                    cascade.AddStyleSheet(&styleSheet->GetPropertyTable());
                }
            }

            SetupControlFromCascade(control, cascade);
        }
    }

    void UIStyleSheetSystem::SortStyleSheets()
    {
        std::sort(styleSheets.begin(), styleSheets.end(),
            [](const UIStyleSheet* first, const UIStyleSheet* second) {
            return first->GetScore() < second->GetScore();
        });
    }

    bool UIStyleSheetSystem::StyleSheetMatchesControl(UIStyleSheet* styleSheet, UIControl* control)
    {
        UIControl* currentControl = control;

        for (const UIStyleSheetSelector& selector : styleSheet->GetSelectorChain())
        {
            if (!currentControl || !SelectorMatchesControl(&selector, currentControl))
                return false;

            currentControl = currentControl->GetParent();
        }

        return true;
    }

    bool UIStyleSheetSystem::SelectorMatchesControl(const UIStyleSheetSelector* selector, UIControl* control)
    {
        if ((selector->name.IsValid() && selector->name != control->GetFastName())
            || (!selector->className.empty() && selector->className != control->GetClassName()))
            return false;

        for (const FastName& clazz : selector->classes)
        {
            if (!control->HasClass(clazz))
                return false;
        }

        return true;
    }

    void UIStyleSheetSystem::SetupControlFromCascade(UIControl* control, const UIStyleSheetCascade& cascade)
    {
        SetupObjectPropertiesFromCascade(control, control->GetTypeInfo(), cascade);
        for (UIComponent* component : control->GetComponents())
        {
            if (!UIComponent::IsMultiple(component->GetType()))
                SetupObjectPropertiesFromCascade(component, component->GetTypeInfo(), cascade);
        }
        
        if (control->GetBackgroundComponentsCount() > 0) // multiple backgrounds are evil
        {
            UIControlBackground* bg = control->GetBackgroundComponent(0);
            SetupObjectPropertiesFromCascade(bg, bg->GetTypeInfo(), cascade);
        }
    }

    template < class T >
    void UIStyleSheetSystem::SetupObjectPropertiesFromCascade(T* object, const InspInfo* typeInfo, const UIStyleSheetCascade& cascade)
    {
        const InspInfo *baseInfo = typeInfo->BaseInfo();
        if (baseInfo)
            SetupObjectPropertiesFromCascade(object, baseInfo, cascade);

        for (int32 i = 0; i < typeInfo->MembersCount(); i++)
        {
            const InspMember *member = typeInfo->Member(i);

            const VariantType* value = cascade.GetProperty(member->Name());

            if (value)
                member->SetValue(object, *value);
        }
    }
}
