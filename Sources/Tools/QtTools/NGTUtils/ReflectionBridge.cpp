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

#include "ReflectionBridge.h"
#include "FileSystem/KeyedArchive.h"
#include "Utils/Utils.h"

#include "wg_types/vector2.hpp"
#include "wg_types/vector3.hpp"
#include "wg_types/vector4.hpp"

#include "VariantConverter.h"
#include "NGTCollectionsImpl.h"

namespace DAVA
{
namespace
{
class EnumGenerator : public IEnumGenerator
{
public:
    EnumGenerator(const InspMember* insp)
        : memberInsp(insp)
    {
    }

    Collection getCollection(const ObjectHandle& provider, const IDefinitionManager& definitionManager) override
    {
        InspDesc const& desc = memberInsp->Desc();

        DVASSERT(desc.enumMap != nullptr);

        using TCollection = std::map<int, char const*>;
        TCollection* enumMap = new TCollection;

        Collection result;
        for (size_t i = 0; i < desc.enumMap->GetCount(); ++i)
        {
            int value = 0;
            if (desc.enumMap->GetValue(i, value))
                enumMap->insert(std::make_pair(i, desc.enumMap->ToString(value)));
        }

        return Collection(*enumMap);
    }

private:
    const InspMember* memberInsp;
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

} // namespace

NGTTypeDefinition::NGTTypeDefinition(const InspInfo* info_)
    : info(info_)
{
    DVASSERT(info != nullptr);
    
    displayName = StringToWString(info->Name().c_str());
    metaHandle = MetaDisplayName(displayName.c_str());

    const MetaInfo* objectType = info->Type();

    for (int i = 0; i < info->MembersCount(); ++i)
    {
        const InspMember* member = info->Member(i);
        const MetaInfo* metaInfo = member->Type();

        int memberFlags = member->Flags();
        if ((memberFlags & I_VIEW) == 0)
            continue;

        const InspMemberDynamic* dynamicMember = member->Dynamic();
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
    const InspInfo* parentInfo = info->BaseInfo();
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

NGTMemberProperty::NGTMemberProperty(const InspMember* member, const MetaInfo* objectType_)
    : BaseProperty(member->Name().c_str(), member->Type()->GetTypeName())
    , objectType(objectType_)
    , memberInsp(member)
    , metaBase(MetaNone())
{
    DVASSERT(objectType != nullptr);
    DVASSERT(memberInsp != nullptr);

    const InspDesc& desc = memberInsp->Desc();
    if (desc.enumMap != nullptr)
        metaBase = metaBase + MetaEnum(new EnumGenerator(memberInsp));

    const MetaInfo* metaType = member->Type();

    if ((metaType->IsPointer() && metaType->GetIntrospection() == nullptr) ||
        metaType == MetaInfo::Instance<FastName>() ||
        metaType == MetaInfo::Instance<FilePath>() ||
        metaType == MetaInfo::Instance<Matrix2>() ||
        metaType == MetaInfo::Instance<Matrix3>() || 
        metaType == MetaInfo::Instance<Matrix4>() ||
        metaType == MetaInfo::Instance<AABBox3>())
    {
        setType(TypeId(MetaInfo::Instance<String>()->GetTypeName()));
    }
    else if (metaType == MetaInfo::Instance<Vector2>())
        setType(TypeId(getClassIdentifier<::Vector2>()));
    else if (metaType == MetaInfo::Instance<Vector3>())
        setType(TypeId(getClassIdentifier<::Vector3>()));
    else if (metaType == MetaInfo::Instance<Color>())
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
        const MetaInfo* memberMetaInfo = memberInsp->Type();
        const InspInfo* fieldIntrospection = memberMetaInfo->GetIntrospection(field);

        if (nullptr != fieldIntrospection && (fieldIntrospection->Type() == MetaInfo::Instance<KeyedArchive>()))
        {
            return Collection(std::make_shared<NGTKeyedArchiveImpl>(reinterpret_cast<KeyedArchive *>(field)));
        }
        // introspection
        else if (nullptr != field && nullptr != fieldIntrospection)
        {
            return CreateObjectHandle(const_cast<IDefinitionManager&>(definitionManager), fieldIntrospection, field);
        }
        else if (memberMetaInfo->IsPointer())
        {
            String pointerValue(64, 0);
            sprintf(&pointerValue[0], "[0x%p] Pointer", field);
            return pointerValue;
        }
        else if (memberInsp->Collection())
        {
            DVASSERT(field != nullptr);
            const InspColl* collection = memberInsp->Collection();
            return Collection(std::make_shared<NGTCollection>(field, collection));
        }
        else if (memberInsp->Dynamic())
        {
            return Variant();
        }

        return VariantConverter::Instance()->Convert(memberInsp->Value(object));
    }

    return Variant();
}

bool NGTMemberProperty::set(const ObjectHandle& pBase, const Variant& v, const IDefinitionManager& definitionManager) const
{
    void* object = UpCast(pBase, definitionManager);
    if (object == nullptr)
        return false;

    const MetaInfo* type = memberInsp->Type();
    if (type->IsPointer() || ((memberInsp->Flags() & I_EDIT) == 0))
        return false;

    VariantType value = VariantConverter::Instance()->Convert(v, type);
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
    return (memberInsp->Flags() & I_EDIT) == 0;
}

bool NGTMemberProperty::isValue() const
{
    return true;
}

void RegisterType(IDefinitionManager& mng, const InspInfo* inspInfo)
{
    if (inspInfo == nullptr)
        return;

    using TDefinitionMap = std::unordered_map<const MetaInfo*, NGTTypeDefinition*>;
    static TDefinitionMap definitionMap;

    const MetaInfo* type = inspInfo->Type();

    TDefinitionMap::iterator definitionIter = definitionMap.find(type);
    if (definitionIter == definitionMap.end())
    {
        definitionIter = definitionMap.emplace(type, new NGTTypeDefinition(inspInfo)).first;
        mng.registerDefinition(definitionIter->second);
        RegisterType(mng, inspInfo->BaseInfo());
    }
}

ObjectHandle CreateObjectHandle(IDefinitionManager& defMng, const InspInfo* fieldInsp, void* field)
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
}