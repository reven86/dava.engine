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


#include "SectionProperty.h"

#include "Model/ControlProperties/ValueProperty.h"

using namespace DAVA;

SectionProperty::SectionProperty(const DAVA::String &sectionName)
    : name(sectionName)
{
    
}

SectionProperty::~SectionProperty()
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        DVASSERT((*it)->GetParent() == this);
        (*it)->SetParent(NULL);
        (*it)->Release();
    }
    children.clear();
}

void SectionProperty::AddProperty(ValueProperty *value)
{
    DVASSERT(value->GetParent() == NULL);
    value->SetParent(this);
    children.push_back(SafeRetain(value));
}

int SectionProperty::GetCount() const
{
    return static_cast<int>(children.size());
}

AbstractProperty *SectionProperty::GetProperty(int index) const
{
    if (0 <= index && index < children.size())
        return children[index];

    DVASSERT(false);
    return nullptr;
}

void SectionProperty::Refresh()
{
    for (ValueProperty *prop : children)
        prop->Refresh();
}

const DAVA::String & SectionProperty::GetName() const
{
    return name;
}

ValueProperty *SectionProperty::FindProperty(const DAVA::InspMember *member) const
{
    for (auto child : children)
    {
        if (child->IsSameMember(member))
            return child;
    }
    return nullptr;
}
