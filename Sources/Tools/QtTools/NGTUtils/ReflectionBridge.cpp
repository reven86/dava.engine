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

#include "VariantConverter.h"
#include "NGTCollectionsImpl.h"
#include "ReflectionBridge.h"

#include "FileSystem/KeyedArchive.h"
#include "Utils/Utils.h"
#include "Math/AABBox3.h"

#include "wg_types/vector2.hpp"
#include "wg_types/vector3.hpp"
#include "wg_types/vector4.hpp"

namespace NGTLayer
{
class EnumGenerator : public IEnumGenerator
{
public:
    EnumGenerator(const DAVA::InspMember* insp)
        : memberInsp(insp)
    {
    }

    Collection getCollection(const ObjectHandle& provider, const IDefinitionManager& definitionManager) override
    {
        DAVA::InspDesc const& desc = memberInsp->Desc();

        DVASSERT(desc.enumMap != nullptr);

        using TCollection = std::map<int, char const*>;
        TCollection* enumMap = new TCollection;

        for (size_t i = 0; i < desc.enumMap->GetCount(); ++i)
        {
            int value = 0;
            if (desc.enumMap->GetValue(i, value))
                enumMap->insert(std::make_pair(i, desc.enumMap->ToString(value)));
        }

        return Collection(*enumMap);
    }

private:
    const DAVA::InspMember* memberInsp;
};

class DavaObjectStorage : public IObjectHandleStorage
{
public:
    DavaObjectStorage(void* object_, const char* typeName)
        : object(object_)
        , typeId(typeName)
    {
        DVASSERT(object != nullptr);
    }

    void* data() const override
    {
        return object;
    }

    TypeId type() const override
    {
        return typeId;
    }

    bool getId(RefObjectId& id) const override
    {
        return false;
    }

private:
    void* object;
    TypeId typeId;
};

NGTTypeDefinition::NGTTypeDefinition(const DAVA::InspInfo* info_)
    : info(info_)
{
    DVASSERT(info != nullptr);

    displayName = DAVA::StringToWString(info->Name().c_str());
    metaHandle = MetaDisplayName(displayName.c_str());

    const DAVA::MetaInfo* objectType = info->Type();

    for (int i = 0; i < info->MembersCount(); ++i)
    {
        const DAVA::InspMember* member = info->Member(i);
        const DAVA::MetaInfo* metaInfo = member->Type();

        int memberFlags = member->Flags();
        if ((memberFlags & DAVA::I_VIEW) == 0)
            continue;

        properties.addProperty(IBasePropertyPtr(new NGTMemberProperty(member, objectType)));
    }
}

bool NGTTypeDefinition::isGeneric() const
{
    return false;
}

MetaHandle NGTTypeDefinition::getMetaData() const
{
    return metaHandle;
}

bool NGTTypeDefinition::isAbstract() const
{
    return false;
}

const char* NGTTypeDefinition::getName() const
{
    return info->Type()->GetTypeName();
}

const char* NGTTypeDefinition::getParentName() const
{
    const DAVA::InspInfo* parentInfo = info->BaseInfo();
    if (parentInfo != nullptr)
        parentInfo->Type()->GetTypeName();

    return nullptr;
}

ObjectHandle NGTTypeDefinition::create(const IClassDefinition& classDefinition) const
{
    throw std::logic_error("The method or operation is not implemented.");
    return ObjectHandle();
}

void* NGTTypeDefinition::upCast(void* object) const
{
    if (info->BaseInfo() != nullptr)
        return object;

    return nullptr;
}

PropertyIteratorImplPtr NGTTypeDefinition::getPropertyIterator() const
{
    return properties.getIterator();
}

IClassDefinitionModifier* NGTTypeDefinition::getDefinitionModifier() const
{
    return nullptr;
}

NGTMemberProperty::NGTMemberProperty(const DAVA::InspMember* member, const DAVA::MetaInfo* objectType_)
    : BaseProperty(member->Name().c_str(), member->Type()->GetTypeName())
    , objectType(objectType_)
    , memberInsp(member)
    , metaBase(MetaNone())
{
    DVASSERT(objectType != nullptr);
    DVASSERT(memberInsp != nullptr);

    if ((memberInsp->Flags() & DAVA::I_EDIT) == 0)
        metaBase = metaBase + MetaReadOnly();

    const DAVA::InspDesc& desc = memberInsp->Desc();
    if (desc.enumMap != nullptr)
        metaBase = metaBase + MetaEnum(new EnumGenerator(memberInsp));

    const DAVA::MetaInfo* metaType = member->Type();

    if ((metaType->IsPointer() && metaType->GetIntrospection() == nullptr) ||
        metaType == DAVA::MetaInfo::Instance<DAVA::FastName>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::FilePath>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::Matrix2>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::Matrix3>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::Matrix4>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::AABBox3>())
    {
        setType(TypeId(DAVA::MetaInfo::Instance<DAVA::String>()->GetTypeName()));
    }
    else if (metaType == DAVA::MetaInfo::Instance<DAVA::Vector2>())
        setType(TypeId(getClassIdentifier<::Vector2>()));
    else if (metaType == DAVA::MetaInfo::Instance<DAVA::Vector3>())
        setType(TypeId(getClassIdentifier<::Vector3>()));
    else if (metaType == DAVA::MetaInfo::Instance<DAVA::Color>())
    {
        setType(TypeId(getClassIdentifier<::Vector4>()));
        metaBase = metaBase + MetaColor();
    }
}

Variant NGTMemberProperty::get(const ObjectHandle& pBase, const IDefinitionManager& definitionManager) const
{
    void* object = UpCast(pBase, definitionManager);
    if (object != nullptr)
    {
        void* field = memberInsp->Data(object);
        const DAVA::MetaInfo* memberMetaInfo = memberInsp->Type();
        const DAVA::InspInfo* fieldIntrospection = memberMetaInfo->GetIntrospection(field);

        if (nullptr != fieldIntrospection && (fieldIntrospection->Type() == DAVA::MetaInfo::Instance<DAVA::KeyedArchive>()))
        {
            return Collection(std::make_shared<NGTKeyedArchiveImpl>(reinterpret_cast<DAVA::KeyedArchive*>(field)));
        }
        // introspection
        else if (nullptr != field && nullptr != fieldIntrospection)
        {
            return CreateObjectHandle(const_cast<IDefinitionManager&>(definitionManager), fieldIntrospection, field);
        }
        else if (memberMetaInfo->IsPointer())
        {
            DAVA::String pointerValue(64, 0);
            sprintf(&pointerValue[0], "[0x%p] Pointer", field);
            return pointerValue;
        }
        else if (memberInsp->Collection())
        {
            DVASSERT(field != nullptr);
            const DAVA::InspColl* collection = memberInsp->Collection();
            return Collection(std::make_shared<NGTCollection>(field, collection));
        }
        else if (memberInsp->Dynamic())
        {
            return Variant();
        }

        return VariantConverter::Convert(memberInsp->Value(object));
    }

    return Variant();
}

bool NGTMemberProperty::set(const ObjectHandle& pBase, const Variant& v, const IDefinitionManager& definitionManager) const
{
    void* object = UpCast(pBase, definitionManager);
    if (object == nullptr)
        return false;

    const DAVA::MetaInfo* type = memberInsp->Type();
    if (type->IsPointer() || ((memberInsp->Flags() & DAVA::I_EDIT) == 0))
        return false;

    DAVA::VariantType value = VariantConverter::Convert(v, type);
    memberInsp->SetValue(object, value);
    return true;
}

MetaHandle NGTMemberProperty::getMetaData() const
{
    return metaBase;
}

void* NGTMemberProperty::UpCast(ObjectHandle const& pBase, const IDefinitionManager& definitionManager) const
{
    TypeId srcID = pBase.type();
    TypeId dstID(objectType->GetTypeName());
    return reflectedCast(pBase.data(), srcID, dstID, definitionManager);
}

bool NGTMemberProperty::readOnly() const
{
    DVASSERT(memberInsp != nullptr);
    return (memberInsp->Flags() & DAVA::I_EDIT) == 0;
}

bool NGTMemberProperty::isValue() const
{
    return true;
}

void RegisterType(IDefinitionManager& mng, const DAVA::InspInfo* inspInfo)
{
    if (inspInfo == nullptr)
        return;

    static DAVA::UnorderedMap<const DAVA::MetaInfo*, NGTTypeDefinition*> definitionMap;

    const DAVA::MetaInfo* type = inspInfo->Type();

    DAVA::UnorderedMap<const DAVA::MetaInfo*, NGTTypeDefinition*>::iterator definitionIter = definitionMap.find(type);
    if (definitionIter == definitionMap.end())
    {
        definitionIter = definitionMap.emplace(type, new NGTTypeDefinition(inspInfo)).first;
        mng.registerDefinition(definitionIter->second);
        RegisterType(mng, inspInfo->BaseInfo());
    }
}

ObjectHandle CreateObjectHandle(IDefinitionManager& defMng, const DAVA::InspInfo* fieldInsp, void* field)
{
    const char* typeName = fieldInsp->Type()->GetTypeName();
    IClassDefinition* def = defMng.getDefinition(typeName);
    if (def == nullptr)
    {
        RegisterType((defMng), fieldInsp);
    }

    std::shared_ptr<IObjectHandleStorage> storage(new DavaObjectStorage(field, typeName));
    return ObjectHandle(storage);
}
} // namespace NGTLayer