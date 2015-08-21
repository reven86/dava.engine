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


#include "Base/IntrospectionBase.h"
#include "Base/Meta.h"

namespace DAVA
{

InspMember::InspMember(const char *_name, const InspDesc &_desc, const size_t _offset, const MetaInfo *_type, int _flags /* = 0 */)
: name(_name), desc(_desc), offset(_offset), type(_type), flags(_flags), parentInsp(nullptr)
{ }

const FastName& InspMember::Name() const
{
    return name;
}

const InspDesc& InspMember::Desc() const
{
	return desc;
}

const MetaInfo* InspMember::Type() const
{
	return type;
}

void* InspMember::Pointer(void *object) const
{
    return OffsetPointer<void>(object, offset);
}

void* InspMember::Data(void *object) const
{
	if(type->IsPointer())
	{
		return *static_cast<void **>(Pointer(object));
	}
	else
	{
		return Pointer(object);
	}
}

VariantType InspMember::Value(void *object) const
{
	return VariantType::LoadData(Pointer(object), type);
}

void InspMember::SetValue(void *object, const VariantType &val) const
{
	VariantType::SaveData(Pointer(object), type, val);
}

void InspMember::SetValueRaw(void *object, void* val) const
{
	DVASSERT(false);
}

const InspColl* InspMember::Collection() const
{
	return nullptr;
}

const InspMemberDynamic* InspMember::Dynamic() const
{
	return nullptr;
}

int InspMember::Flags() const
{
	return flags;
}

void InspMember::ApplyParentInsp(const InspInfo *_parentInsp) const
{
	parentInsp = _parentInsp;
}

};
