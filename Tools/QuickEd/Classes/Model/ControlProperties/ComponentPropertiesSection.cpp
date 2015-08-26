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


#include "ComponentPropertiesSection.h"

#include "IntrospectionProperty.h"
#include "PropertyVisitor.h"

#include "UI/UIControl.h"

using namespace DAVA;

ComponentPropertiesSection::ComponentPropertiesSection(DAVA::UIControl *aControl, DAVA::UIComponent::eType type, int32 _index, const ComponentPropertiesSection *sourceSection, eCloneType cloneType)
    : SectionProperty("")
    , control(SafeRetain(aControl))
    , component(nullptr)
    , index(_index)
    , prototypeSection(nullptr) // weak
{
    component = UIComponent::CreateByType(type);
    DVASSERT(component);

    if (sourceSection && cloneType == CT_INHERIT)
    {
        prototypeSection = sourceSection; // weak
    }

    RefreshName();

    const InspInfo *insp = component->GetTypeInfo();
    for (int j = 0; j < insp->MembersCount(); j++)
    {
        const InspMember *member = insp->Member(j);
        
        const IntrospectionProperty *sourceProp = sourceSection == nullptr ? nullptr : sourceSection->FindProperty(member);
        IntrospectionProperty *prop = new IntrospectionProperty(component, member, sourceProp, cloneType);
        AddProperty(prop);
        SafeRelease(prop);
    }
}

ComponentPropertiesSection::~ComponentPropertiesSection()
{
    SafeRelease(control);
    SafeRelease(component);
    prototypeSection = nullptr; // weak
}

UIComponent *ComponentPropertiesSection::GetComponent() const
{
    return component;
}

DAVA::uint32 ComponentPropertiesSection::GetComponentType() const
{
    return component->GetType();
}

void ComponentPropertiesSection::AttachPrototypeSection(ComponentPropertiesSection *section)
{
    if (prototypeSection == nullptr)
    {
        prototypeSection = section;
        const InspInfo *insp = component->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            ValueProperty *value = FindProperty(member);
            ValueProperty *prototypeValue = prototypeSection->FindProperty(member);
            value->AttachPrototypeProperty(prototypeValue);
        }
    }
    else
    {
        DVASSERT(false);
    }
}

void ComponentPropertiesSection::DetachPrototypeSection(ComponentPropertiesSection *section)
{
    if (prototypeSection == section)
    {
        prototypeSection = nullptr; // weak
        for (int i = 0; i < GetCount(); i++)
        {
            ValueProperty *value = GetProperty(i);
            if (value->GetPrototypeProperty())
            {
                DVASSERT(value->GetPrototypeProperty()->GetParent() == section);
                value->DetachPrototypeProperty(value->GetPrototypeProperty());
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
    else
    {
        DVASSERT(false);
    }
}

bool ComponentPropertiesSection::HasChanges() const
{
    return SectionProperty::HasChanges();
}

uint32 ComponentPropertiesSection::GetFlags() const
{
    bool readOnly = IsReadOnly();
    
    uint32 flags = 0;
    
    if (!readOnly && prototypeSection == nullptr)
        flags |= EF_CAN_REMOVE;
    
    if (prototypeSection)
        flags |= EF_INHERITED;
    
    return flags;
}

void ComponentPropertiesSection::InstallComponent()
{
    if (control->GetComponent(component->GetType(), 0) != component)
    {
        control->InsertComponentAt(component, index);
    }
}

void ComponentPropertiesSection::UninstallComponent()
{
    UIComponent *installedComponent = control->GetComponent(component->GetType(), index);
    if (installedComponent)
    {
        DVASSERT(installedComponent == component);
        control->RemoveComponent(component);
    }
}

int32 ComponentPropertiesSection::GetComponentIndex() const
{
    return index;
}

void ComponentPropertiesSection::RefreshIndex()
{
    if (component->GetControl() == control)
    {
        index = control->GetComponentIndex(component);
        RefreshName();
    }
}

void ComponentPropertiesSection::Accept(PropertyVisitor *visitor)
{
    visitor->VisitComponentSection(this);
}

String ComponentPropertiesSection::GetComponentName() const
{
    return GlobalEnumMap<UIComponent::eType>::Instance()->ToString(component->GetType());
}

void ComponentPropertiesSection::RefreshName()
{
    name = GetComponentName();
    if (UIComponent::IsMultiple(component->GetType()))
        name += Format(" [%d]", index);
}
