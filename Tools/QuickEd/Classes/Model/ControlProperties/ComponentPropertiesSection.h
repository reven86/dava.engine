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


#ifndef __QUICKED_COMPONENT_PROPERTIES_SECTION_H__
#define __QUICKED_COMPONENT_PROPERTIES_SECTION_H__

#include "SectionProperty.h"
#include "IntrospectionProperty.h"

#include "UI/Components/UIComponent.h"

namespace DAVA
{
    class UIControl;
}

class ComponentPropertiesSection : public SectionProperty<IntrospectionProperty>
{
public:
    ComponentPropertiesSection(DAVA::UIControl *control, DAVA::UIComponent::eType type, DAVA::int32 index, const ComponentPropertiesSection *sourceSection, eCloneType copyType);
protected:
    virtual ~ComponentPropertiesSection();

public:
    DAVA::UIComponent *GetComponent() const;
    DAVA::uint32 GetComponentType() const;
    
    void AttachPrototypeSection(ComponentPropertiesSection *section);
    void DetachPrototypeSection(ComponentPropertiesSection *section);
    
    bool HasChanges() const override;
    DAVA::uint32 GetFlags() const override;
    
    void InstallComponent();
    void UninstallComponent();
    
    DAVA::int32 GetComponentIndex() const;
    void RefreshIndex();

    void Accept(PropertyVisitor *visitor) override;

    DAVA::String GetComponentName() const;

private:
    void RefreshName();
    
private:
    DAVA::UIControl *control;
    DAVA::UIComponent *component;
    DAVA::int32 index;
    const ComponentPropertiesSection *prototypeSection;
};

#endif // __QUICKED_COMPONENT_PROPERTIES_SECTION_H__
