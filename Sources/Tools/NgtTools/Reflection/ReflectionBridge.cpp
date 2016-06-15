#include "VariantConverter.h"
#include "NGTCollectionsImpl.h"
#include "ReflectionBridge.h"

#include "FileSystem/KeyedArchive.h"
#include "Utils/Utils.h"
#include "Math/AABBox3.h"

#include <wg_types/vector2.hpp>
#include <wg_types/vector3.hpp>
#include <wg_types/vector4.hpp>

namespace NGTLayer
{
class EnumGenerator : public wgt::IEnumGenerator
{
public:
    EnumGenerator(const DAVA::InspMember* insp)
        : memberInsp(insp)
    {
    }

    wgt::Collection getCollection(const wgt::ObjectHandle& provider, const wgt::IDefinitionManager& definitionManager) override
    {
        DAVA::InspDesc const& desc = memberInsp->Desc();

        DVASSERT(desc.enumMap != nullptr);

        using TCollection = std::map<int, char const*>;
        TCollection* enumMap = new TCollection;

        for (size_t i = 0; i < desc.enumMap->GetCount(); ++i)
        {
            int value = 0;
            if (desc.enumMap->GetValue(i, value))
                enumMap->insert(std::make_pair(static_cast<int>(i), desc.enumMap->ToString(value)));
        }

        return wgt::Collection(*enumMap);
    }

private:
    const DAVA::InspMember* memberInsp;
};

class DavaObjectStorage : public wgt::IObjectHandleStorage
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

    wgt::TypeId type() const override
    {
        return typeId;
    }

    bool getId(wgt::RefObjectId& id) const override
    {
        return false;
    }

private:
    void* object;
    wgt::TypeId typeId;
};

NGTTypeDefinition::NGTTypeDefinition(const DAVA::InspInfo* info_)
    : info(info_)
{
    DVASSERT(info != nullptr);

    displayName = DAVA::StringToWString(info->Name().c_str());
    metaHandle = wgt::MetaDisplayName(displayName.c_str());

    const DAVA::MetaInfo* objectType = info->Type();

    for (int i = 0; i < info->MembersCount(); ++i)
    {
        const DAVA::InspMember* member = info->Member(i);
        const DAVA::MetaInfo* metaInfo = member->Type();

        int memberFlags = member->Flags();
        if ((memberFlags & DAVA::I_VIEW) == 0)
            continue;

        properties.addProperty(wgt::IBasePropertyPtr(new NGTMemberProperty(member, objectType)));
    }
}

bool NGTTypeDefinition::isGeneric() const
{
    return false;
}

wgt::MetaHandle NGTTypeDefinition::getMetaData() const
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

wgt::ObjectHandle NGTTypeDefinition::create(const wgt::IClassDefinition& classDefinition) const
{
    throw std::logic_error("The method or operation is not implemented.");
    return wgt::ObjectHandle();
}

void* NGTTypeDefinition::upCast(void* object) const
{
    if (info->BaseInfo() != nullptr)
        return object;

    return nullptr;
}

wgt::PropertyIteratorImplPtr NGTTypeDefinition::getPropertyIterator() const
{
    return properties.getIterator();
}

wgt::IClassDefinitionModifier* NGTTypeDefinition::getDefinitionModifier() const
{
    return nullptr;
}

NGTMemberProperty::NGTMemberProperty(const DAVA::InspMember* member, const DAVA::MetaInfo* objectType_)
    : BaseProperty(member->Name().c_str(), member->Type()->GetTypeName())
    , objectType(objectType_)
    , memberInsp(member)
    , metaBase(wgt::MetaNone())
{
    DVASSERT(objectType != nullptr);
    DVASSERT(memberInsp != nullptr);

    if ((memberInsp->Flags() & DAVA::I_EDIT) == 0)
        metaBase = metaBase + wgt::MetaReadOnly();

    const DAVA::InspDesc& desc = memberInsp->Desc();
    if (desc.enumMap != nullptr)
        metaBase = metaBase + wgt::MetaEnum(wgt::IEnumGeneratorPtr(new EnumGenerator(memberInsp)));

    const DAVA::MetaInfo* metaType = member->Type();

    if ((metaType->IsPointer() && metaType->GetIntrospection() == nullptr) ||
        metaType == DAVA::MetaInfo::Instance<DAVA::FastName>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::FilePath>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::Matrix2>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::Matrix3>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::Matrix4>() ||
        metaType == DAVA::MetaInfo::Instance<DAVA::AABBox3>())
    {
        setType(wgt::TypeId(DAVA::MetaInfo::Instance<DAVA::String>()->GetTypeName()));
    }
    else if (metaType == DAVA::MetaInfo::Instance<DAVA::Vector2>())
        setType(wgt::TypeId(wgt::getClassIdentifier<wgt::Vector2>()));
    else if (metaType == DAVA::MetaInfo::Instance<DAVA::Vector3>())
        setType(wgt::TypeId(wgt::getClassIdentifier<wgt::Vector3>()));
    else if (metaType == DAVA::MetaInfo::Instance<DAVA::Color>())
    {
        setType(wgt::TypeId(wgt::getClassIdentifier<wgt::Vector4>()));
        metaBase = metaBase + wgt::MetaColor();
    }
}

wgt::Variant NGTMemberProperty::get(const wgt::ObjectHandle& pBase, const wgt::IDefinitionManager& definitionManager) const
{
    void* object = UpCast(pBase, definitionManager);
    if (object != nullptr)
    {
        void* field = memberInsp->Data(object);
        const DAVA::MetaInfo* memberMetaInfo = memberInsp->Type();
        const DAVA::InspInfo* fieldIntrospection = memberMetaInfo->GetIntrospection(field);

        if (nullptr != fieldIntrospection && (fieldIntrospection->Type() == DAVA::MetaInfo::Instance<DAVA::KeyedArchive>()))
        {
            return wgt::Collection(std::make_shared<NGTKeyedArchiveImpl>(reinterpret_cast<DAVA::KeyedArchive*>(field)));
        }
        // introspection
        else if (nullptr != field && nullptr != fieldIntrospection)
        {
            return CreateObjectHandle(const_cast<wgt::IDefinitionManager&>(definitionManager), fieldIntrospection, field);
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
            return wgt::Collection(std::make_shared<NGTCollection>(field, collection));
        }
        else if (memberInsp->Dynamic())
        {
            return wgt::Variant();
        }

        return VariantConverter::Convert(memberInsp->Value(object));
    }

    return wgt::Variant();
}

bool NGTMemberProperty::set(const wgt::ObjectHandle& pBase, const wgt::Variant& v, const wgt::IDefinitionManager& definitionManager) const
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

wgt::MetaHandle NGTMemberProperty::getMetaData() const
{
    return metaBase;
}

void* NGTMemberProperty::UpCast(wgt::ObjectHandle const& pBase, const wgt::IDefinitionManager& definitionManager) const
{
    wgt::TypeId srcID = pBase.type();
    wgt::TypeId dstID(objectType->GetTypeName());
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

void RegisterType(wgt::IDefinitionManager& mng, const DAVA::InspInfo* inspInfo)
{
    if (inspInfo == nullptr)
        return;

    static DAVA::UnorderedSet<const DAVA::MetaInfo*> definitionMap;

    const DAVA::MetaInfo* type = inspInfo->Type();

    DAVA::UnorderedSet<const DAVA::MetaInfo*>::iterator definitionIter = definitionMap.find(type);
    if (definitionIter == definitionMap.end())
    {
        definitionMap.insert(type);
        mng.registerDefinition(std::unique_ptr<wgt::IClassDefinitionDetails>(new NGTTypeDefinition(inspInfo)));
        RegisterType(mng, inspInfo->BaseInfo());
    }
}

wgt::ObjectHandle CreateObjectHandle(wgt::IDefinitionManager& defMng, const DAVA::InspInfo* fieldInsp, void* field)
{
    const char* typeName = fieldInsp->Type()->GetTypeName();
    wgt::IClassDefinition* def = defMng.getDefinition(typeName);
    if (def == nullptr)
    {
        RegisterType((defMng), fieldInsp);
    }

    std::shared_ptr<wgt::IObjectHandleStorage> storage(new DavaObjectStorage(field, typeName));
    return wgt::ObjectHandle(storage);
}
} // namespace NGTLayer