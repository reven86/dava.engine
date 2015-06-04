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


#include "ControlPropertiesSection.h"

#include "UI/UIControl.h"
#include "ValueProperty.h"
#include "LocalizedTextValueProperty.h"
#include "FontValueProperty.h"

using namespace DAVA;

ControlPropertiesSection::ControlPropertiesSection(DAVA::UIControl *aControl, const DAVA::InspInfo *typeInfo, const ControlPropertiesSection *sourceSection, eCloneType cloneType)
    : SectionProperty(typeInfo->Name())
    , control(SafeRetain(aControl))
{
    for (int i = 0; i < typeInfo->MembersCount(); i++)
    {
        const InspMember *member = typeInfo->Member(i);
        if ((member->Flags() & I_EDIT) != 0)
        {
            String memberName = member->Name();
            
            ValueProperty *sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindProperty(member);

            ValueProperty *prop = nullptr;
            //TODO: move it to fabric class
            if (strcmp(member->Name(), "text") == 0)
            {
                prop = new LocalizedTextValueProperty(control, member, dynamic_cast<LocalizedTextValueProperty*>(sourceProperty), cloneType);
            }
            else if (strcmp(member->Name(), "font") == 0)
            {
                prop = new FontValueProperty(control, member, dynamic_cast<FontValueProperty*>(sourceProperty), cloneType);
            }
            else
            {
                prop = new IntrospectionProperty(control, member, dynamic_cast<IntrospectionProperty *>(sourceProperty), cloneType);
            }

            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

ControlPropertiesSection::~ControlPropertiesSection()
{
    SafeRelease(control);
}
