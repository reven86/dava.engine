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


#ifndef __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
#define __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__

#include "IntrospectionProperty.h"

class LocalizedTextValueProperty : public IntrospectionProperty
{
public:
    LocalizedTextValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, const IntrospectionProperty *sourceProperty, eCloneType cloneType);

protected:
    virtual ~LocalizedTextValueProperty();
    
public:
    void Refresh(DAVA::int32 refreshFlags) override;

    DAVA::VariantType GetValue() const override;
protected:
    void ApplyValue(const DAVA::VariantType &value) override;
    
protected:
    DAVA::WideString text;
};

#endif // __UI_EDITOR_LOCALIZED_TEXT_VALUE_PROPERTY__
