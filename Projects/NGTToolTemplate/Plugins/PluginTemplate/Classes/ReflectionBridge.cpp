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

#include "Base/FastName.h"
#include "Math/Color.h"
#include "Math/AABBox3.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/KeyedArchive.h"

#include "wg_types/vector2.hpp"
#include "wg_types/vector3.hpp"
#include "wg_types/vector4.hpp"

#include "Utils/Utils.h"

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

static uint8 maxChannelValue = 255;

template <typename T>
VariantType VtoDV(Variant const& v)
{
    T value = T();
    DVVERIFY(v.tryCast(value));

    return VariantType(value);
}

template <>
VariantType VtoDV<void>(Variant const& /*v*/)
{
    return VariantType();
}

template <>
VariantType VtoDV<FastName>(Variant const& v)
{
    String strValue;
    DVVERIFY(v.tryCast(strValue));
    return VariantType(FastName(strValue));
}

template <>
VariantType VtoDV<DAVA::Vector2>(Variant const& v)
{
    ::Vector2 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(DAVA::Vector2(value.x, value.y));
}

template <>
VariantType VtoDV<DAVA::Vector3>(Variant const& v)
{
    ::Vector3 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(DAVA::Vector3(value.x, value.y, value.z));
}

template <>
VariantType VtoDV<DAVA::Vector4>(Variant const& v)
{
    ::Vector4 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(DAVA::Vector4(value.x, value.y, value.z, value.w));
}

template <>
VariantType VtoDV<Color>(Variant const& v)
{
    ::Vector4 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(DAVA::Color(value.x / maxChannelValue, value.y / maxChannelValue,
                                   value.z / maxChannelValue, value.w / maxChannelValue));
}

template <>
VariantType VtoDV<FilePath>(Variant const& v)
{
    String filePath;
    DVVERIFY(v.tryCast(filePath));
    return VariantType(FilePath(filePath));
}

Variant DVtoV_void(VariantType const& v)
{
    return Variant();
}
Variant DVtoV_bool(VariantType const& v)
{
    return Variant(v.AsBool());
}
Variant DVtoV_int32(VariantType const& v)
{
    return Variant(v.AsInt32());
}
Variant DVtoV_float(VariantType const& v)
{
    return Variant(v.AsFloat());
}
Variant DVtoV_string(VariantType const& v)
{
    return Variant(v.AsString());
}
Variant DVtoV_wideString(VariantType const& v)
{
    return Variant(v.AsWideString());
}
Variant DVtoV_int64(VariantType const& v)
{
    return Variant(v.AsInt64());
}
Variant DVtoV_uint32(VariantType const& v)
{
    return Variant(v.AsUInt32());
}
Variant DVtoV_uint64(VariantType const& v)
{
    return Variant(v.AsUInt64());
}
Variant DVtoV_vector2(VariantType const& v)
{
    DAVA::Vector2 davaVec = v.AsVector2();
    return Variant(::Vector2(davaVec.x, davaVec.y));
}
Variant DVtoV_vector3(VariantType const& v)
{
    DAVA::Vector3 davaVec = v.AsVector3();
    return Variant(::Vector3(davaVec.x, davaVec.y, davaVec.z));
}
Variant DVtoV_vector4(VariantType const& v)
{
    DAVA::Vector4 davaVec = v.AsVector4();
    return Variant(::Vector4(davaVec.x, davaVec.y, davaVec.z, davaVec.w));
}
Variant DVtoV_matrix2(VariantType const& v)
{
    return Variant(); // Variant(v.AsMatrix2());
}
Variant DVtoV_matrix3(VariantType const& v)
{
    return Variant(); // Variant(v.AsMatrix3());
}
Variant DVtoV_matrix4(VariantType const& v)
{
    return Variant(); //Variant(v.AsMatrix4());
}
Variant DVtoV_color(VariantType const& v)
{
    DAVA::Color davaVec = v.AsColor();
    return Variant(::Vector4(davaVec.r * maxChannelValue, davaVec.g * maxChannelValue,
                             davaVec.b * maxChannelValue, davaVec.a * maxChannelValue));
}
Variant DVtoV_fastName(VariantType const& v)
{
    return Variant(v.AsFastName().c_str());
}
Variant DVtoV_aabbox3(VariantType const& v)
{
    return Variant(); // Variant(v.AsAABBox3());
}
Variant DVtoV_filePath(VariantType const& v)
{
    return Variant(v.AsFilePath().GetAbsolutePathname());
}

class DavaObjectStorage : public IObjectHandleStorage
{
public:
    DavaObjectStorage(void* object_, const MetaInfo* type_, const IClassDefinition* definition_)
        : object(object_)
        , objectType(type_)
        , definition(definition_)
        , typeId(objectType->GetTypeName())
    {
        DVASSERT(object != nullptr);
        DVASSERT(objectType != nullptr);
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

    const IClassDefinition* getDefinition(const IDefinitionManager& definitionManager) const override
    {
        DVASSERT(definitionManager.getDefinition(type().getName()) == definition);
        return definition;
    }

    void throwBase() const override
    {
        throw object;
    }

private:
    void* object;
    const MetaInfo* objectType;
    const IClassDefinition* definition;
    TypeId typeId;
};

ObjectHandle CreateDavaReflectedProperty(IDefinitionManager& defMng, const InspInfo* fieldIntp, void* field)
{
    const char* typeName = fieldIntp->Type()->GetTypeName();
    IClassDefinition* def = defMng.getDefinition(typeName);
    if (def == nullptr)
    {
        RegisterDavaType((defMng), fieldIntp);
        def = defMng.getDefinition(typeName);
    }

    DVASSERT(def != nullptr);
    return def->getBaseProvider(field);
}

class CollectionImpl : public CollectionImplBase
{
public:
    class Iterator : public CollectionIteratorImplBase
    {
    public:
        static const uint32 END_ITERATOR_POSITION = static_cast<uint32>(-1);

        Iterator(void* object_, const InspColl* collection, uint32 linearKey_)
            : object(object_)
            , collectionImpl(collection)
            , linearKey(linearKey_)
        {
            iterator = collectionImpl->Begin(object);
            uint32 counter = linearKey;
            while (counter > 0 && iterator != nullptr)
            {
                iterator = collectionImpl->Next(iterator);
                --counter;
            }
        }

        ~Iterator()
        {
            collectionImpl->Finish(iterator);
        }

        Variant key() const override
        {
            const void* key = collectionImpl->ItemKeyData(iterator);
            if (key == nullptr)
            {
                /*const InspInfo * inspInfo = collectionImpl->ItemType()->GetIntrospection();
                if (inspInfo != nullptr)
                    return Variant(inspInfo->Name().c_str());*/
                return Variant(linearKey);
            }

            DVASSERT(collectionImpl->ItemKeyType() != nullptr);
            return VariantConverter::Instance().Convert(VariantType::LoadData(key, collectionImpl->ItemKeyType()));
        }

        Variant value() const override
        {
            const MetaInfo* valueTypeInfo = collectionImpl->ItemType();
            DVASSERT(valueTypeInfo != nullptr);
            if (collectionImpl->ItemType()->GetIntrospection() != nullptr)
            {
                void* itemData = collectionImpl->ItemData(iterator);
                const InspInfo* itemInsp = valueTypeInfo->GetIntrospection(itemData);

                if (itemData != nullptr && itemInsp != nullptr)
                {
                    IDefinitionManager* defMng = Context::queryInterface<IDefinitionManager>();
                    DVASSERT(defMng != nullptr);
                    return CreateDavaReflectedProperty(*defMng, itemInsp, itemData);
                }
            }

            void* valuePointer = collectionImpl->ItemPointer(iterator);
            DVASSERT(valuePointer != nullptr);

            return VariantConverter::Instance().Convert(VariantType::LoadData(valuePointer, valueTypeInfo));
        }

        bool setValue(const Variant& v) const override
        {
            void* valuePointer = collectionImpl->ItemPointer(iterator);
            DVASSERT(valuePointer != nullptr);
            DVASSERT(collectionImpl->ItemType() != nullptr);

            VariantType value = VariantConverter::Instance().Convert(v, collectionImpl->ItemType());

            if (value.GetType() == VariantType::TYPE_NONE)
                return false;

            VariantType::SaveData(valuePointer, collectionImpl->ItemType(), value);

            return true;
        }

        void inc() override
        {
            iterator = collectionImpl->Next(iterator);
            ++linearKey;
        }

        bool equals(const CollectionIteratorImplBase& that) const override
        {
            DVASSERT(dynamic_cast<const Iterator*>(&that) != nullptr);
            const Iterator& thatIter = static_cast<const Iterator&>(that);
            return object == thatIter.object &&
            collectionImpl == thatIter.collectionImpl &&
            iterator == thatIter.iterator &&
            linearKey == thatIter.linearKey;
        }

        CollectionIteratorImplPtr clone() const override
        {
            return CollectionIteratorImplPtr(new Iterator(object, collectionImpl, linearKey));
        }

        bool isValid() const
        {
            return iterator != nullptr;
        }

    private:
        void* object;
        const InspColl* collectionImpl;
        InspColl::Iterator iterator = nullptr;
        uint32 linearKey = 0;
    };

    CollectionImpl(void* object_, const InspColl* collectionImpl_)
        : object(object_)
        , collectionImpl(collectionImpl_)
        , valueT("", 0)
        , keyT("", 0)
    {
        const MetaInfo* valueTypeInfo = collectionImpl->ItemType();
        DVASSERT(valueTypeInfo != nullptr);
        valueT = TypeId(valueTypeInfo->GetTypeName());
        keyT = TypeId(getKeyType()->GetTypeName());
    }

    bool empty() const override
    {
        return size() == 0;
    }

    size_t size() const override
    {
        return collectionImpl->Size(object);
    }

    CollectionIteratorImplPtr begin() override
    {
        return CollectionIteratorImplPtr(new Iterator(object, collectionImpl, 0));
    }

    CollectionIteratorImplPtr end() override
    {
        return CollectionIteratorImplPtr(new Iterator(object, collectionImpl, Iterator::END_ITERATOR_POSITION));
    }

    std::pair<CollectionIteratorImplPtr, bool> get(const Variant& key, GetPolicy policy) override
    {
        DVASSERT_MSG(policy != CollectionImplBase::GET_NEW &&
                     policy != CollectionImplBase::GET_AUTO,
                     "GET_NEW and GET_AUTO policy does't implemented");

        using TRet = std::pair<CollectionIteratorImplPtr, bool>;

        Iterator iter(object, collectionImpl, 0);
        while (iter.isValid())
        {
            if (iter.key() == key)
                break;

            iter.inc();
        }

        if (iter.isValid())
            return TRet(iter.clone(), true);

        return TRet(nullptr, false);
    }

    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& pos) override
    {
        DVASSERT_MSG(false, "Not implemented operation erase");
        return nullptr;
    }

    size_t erase(const Variant& key) override
    {
        DVASSERT_MSG(false, "Not implemented operation erase");
        return 0;
    }

    CollectionIteratorImplPtr erase(const CollectionIteratorImplPtr& first, const CollectionIteratorImplPtr& last) override
    {
        DVASSERT_MSG(false, "Not implemented operation erase");
        return nullptr;
    }

    const TypeId& keyType() const override
    {
        return keyT;
    }

    const TypeId& valueType() const override
    {
        return valueT;
    }

private:
    const MetaInfo* getKeyType() const
    {
        const MetaInfo* keyTypeInfo = collectionImpl->ItemKeyType();
        if (keyTypeInfo == nullptr)
            return MetaInfo::Instance<int32>();
        else
            return keyTypeInfo;
    }

private:
    void* object;
    const InspColl* collectionImpl;
    TypeId valueT;
    TypeId keyT;
};

} // namespace

VariantConverter::VariantConverter()
{
#ifdef __DAVAENGINE_DEBUG__
/*IMetaTypeManager* metaTypeMng = Variant::getMetaTypeManager();
    DVASSERT(metaTypeMng->findType<void>() != nullptr);
    DVASSERT(metaTypeMng->findType<int64>() != nullptr);
    DVASSERT(metaTypeMng->findType<float32>() != nullptr);
    DVASSERT(metaTypeMng->findType<String>() != nullptr);
    DVASSERT(metaTypeMng->findType<WideString>() != nullptr);
    DVASSERT(metaTypeMng->findType<uint64>() != nullptr);
    DVASSERT(metaTypeMng->findType<int64>() != nullptr);
    DVASSERT(metaTypeMng->findType<uint64>() != nullptr);
    DVASSERT(metaTypeMng->findType<Vector2>() != nullptr);
    DVASSERT(metaTypeMng->findType<Vector3>() != nullptr);
    DVASSERT(metaTypeMng->findType<Vector4>() != nullptr);
    DVASSERT(metaTypeMng->findType<Matrix2>() != nullptr);
    DVASSERT(metaTypeMng->findType<Matrix3>() != nullptr);
    DVASSERT(metaTypeMng->findType<Matrix4>() != nullptr);
    DVASSERT(metaTypeMng->findType<Color>() != nullptr);
    DVASSERT(metaTypeMng->findType<FastName>() != nullptr);
    DVASSERT(metaTypeMng->findType<AABBox3>() != nullptr);
    DVASSERT(metaTypeMng->findType<FilePath>() != nullptr);*/
#endif

    using namespace std;
    using namespace std::placeholders;

    convertFunctions[VariantType::TYPE_NONE] = {bind(&VtoDV<void>, _1), bind(&DVtoV_void, _1)};
    convertFunctions[VariantType::TYPE_BOOLEAN] = {bind(&VtoDV<bool>, _1), bind(&DVtoV_bool, _1)};
    convertFunctions[VariantType::TYPE_INT32] = {bind(&VtoDV<int32>, _1), bind(&DVtoV_int32, _1)};
    convertFunctions[VariantType::TYPE_FLOAT] = {bind(&VtoDV<float32>, _1), bind(&DVtoV_float, _1)};
    convertFunctions[VariantType::TYPE_STRING] = {bind(&VtoDV<String>, _1), bind(&DVtoV_string, _1)};
    convertFunctions[VariantType::TYPE_WIDE_STRING] = {bind(&VtoDV<WideString>, _1), bind(&DVtoV_wideString, _1)};
    convertFunctions[VariantType::TYPE_UINT32] = {bind(&VtoDV<uint32>, _1), bind(&DVtoV_uint32, _1)};
    convertFunctions[VariantType::TYPE_INT64] = {bind(&VtoDV<int64>, _1), bind(&DVtoV_int64, _1)};
    convertFunctions[VariantType::TYPE_UINT64] = {bind(&VtoDV<uint64>, _1), bind(&DVtoV_uint64, _1)};
    convertFunctions[VariantType::TYPE_VECTOR2] = {bind(&VtoDV<Vector2>, _1), bind(&DVtoV_vector2, _1)};
    convertFunctions[VariantType::TYPE_VECTOR3] = {bind(&VtoDV<Vector3>, _1), bind(&DVtoV_vector3, _1)};
    convertFunctions[VariantType::TYPE_VECTOR4] = {bind(&VtoDV<Vector4>, _1), bind(&DVtoV_vector4, _1)};
    convertFunctions[VariantType::TYPE_MATRIX2] = {bind(&VtoDV<Matrix2>, _1), bind(&DVtoV_matrix2, _1)};
    convertFunctions[VariantType::TYPE_MATRIX3] = {bind(&VtoDV<Matrix3>, _1), bind(&DVtoV_matrix3, _1)};
    convertFunctions[VariantType::TYPE_MATRIX4] = {bind(&VtoDV<Matrix4>, _1), bind(&DVtoV_matrix4, _1)};
    convertFunctions[VariantType::TYPE_COLOR] = {bind(&VtoDV<Color>, _1), bind(&DVtoV_color, _1)};
    convertFunctions[VariantType::TYPE_FASTNAME] = {bind(&VtoDV<FastName>, _1), bind(&DVtoV_fastName, _1)};
    convertFunctions[VariantType::TYPE_AABBOX3] = {bind(&VtoDV<AABBox3>, _1), bind(&DVtoV_aabbox3, _1)};
    convertFunctions[VariantType::TYPE_FILEPATH] = {bind(&VtoDV<FilePath>, _1), bind(&DVtoV_filePath, _1)};
}

VariantType VariantConverter::Convert(Variant const& v, MetaInfo const* info) const
{
    VariantType::eVariantType davaType = VariantType::TYPE_NONE;
    for (VariantType::PairTypeName const& type : VariantType::variantNamesMap)
    {
        if (type.variantMeta == info)
        {
            davaType = type.variantType;
            break;
        }
    }
    DVASSERT(davaType != VariantType::TYPE_BYTE_ARRAY &&
             davaType != VariantType::TYPE_KEYED_ARCHIVE);

    return convertFunctions[davaType].vToDvFn(v);
}

Variant VariantConverter::Convert(VariantType const& value) const
{
    VariantType::eVariantType davaType = value.GetType();
    DVASSERT(davaType != VariantType::TYPE_BYTE_ARRAY &&
             davaType != VariantType::TYPE_KEYED_ARCHIVE);
    return convertFunctions[davaType].dvToVFn(value);
}

VariantConverter const& VariantConverter::Instance()
{
    static VariantConverter c;
    return c;
}

DavaTypeDefinition::DavaTypeDefinition()
    : metaData(&(MetaNone()))
{
}

bool DavaTypeDefinition::isGeneric() const
{
    return false;
}

const MetaBase* DavaTypeDefinition::getMetaData() const
{
    return metaData.get();
}

ObjectHandle DavaTypeDefinition::createBaseProvider(const ReflectedPolyStruct& polyStruct) const
{
    DVASSERT_MSG(false, ("Dava objects don't derived from ReflectedPolyStruct"));
    return ObjectHandle();
}

IClassDefinitionDetails::CastHelperCache* DavaTypeDefinition::getCastHelperCache() const
{
    return &castHelper;
}

void DavaTypeDefinition::SetDisplayName(const char* name)
{
    displayName = StringToWString(String(name));
    metaData.reset(&(MetaDisplayName(displayName.c_str())));
}

void DavaTypeDefinition::initModifiers(IClassDefinitionModifier& collection, const InspInfo* info)
{
    if (info == nullptr)
        return;

    const MetaInfo* objectType = info->Type();

    for (int i = 0; i < info->MembersCount(); ++i)
    {
        const InspMember* member = info->Member(i);
        const MetaInfo* metaInfo = member->Type();

        int memberFlags = member->Flags();
        if ((memberFlags & I_VIEW) == 0)
            continue;

        const InspMemberDynamic* dynamicMember = member->Dynamic();
        collection.addProperty(new DavaMemberProperty(member, objectType), nullptr);
    }
}

DavaMemberProperty::DavaMemberProperty(const InspMember* member, const MetaInfo* objectType_)
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
        metaType == MetaInfo::Instance<FilePath>())
    {
        setType(TypeId(MetaInfo::Instance<String>()->GetTypeName()));
    }
    else if (metaType == MetaInfo::Instance<DAVA::Vector2>())
        setType(TypeId(getClassIdentifier<::Vector2>()));
    else if (metaType == MetaInfo::Instance<DAVA::Vector3>())
        setType(TypeId(getClassIdentifier<::Vector3>()));
    else if (metaType == MetaInfo::Instance<DAVA::Color>())
    {
        setType(TypeId(getClassIdentifier<::Vector4>()));
        metaBase = metaBase + MetaColor();
    }
}

Variant DavaMemberProperty::get(const ObjectHandle& pBase, const IDefinitionManager& definitionManager) const
{
    void* object = UpCast(pBase, definitionManager);
    if (object)
    {
        void* field = memberInsp->Data(object);
        const MetaInfo* memberMetaInfo = memberInsp->Type();
        const InspInfo* fieldIntrospection = memberMetaInfo->GetIntrospection(field);

        if (nullptr != fieldIntrospection && (fieldIntrospection->Type() == MetaInfo::Instance<KeyedArchive>()))
        {
            return Variant();
        }
        // introspection
        else if (nullptr != field && nullptr != fieldIntrospection)
        {
            return CreateDavaReflectedProperty(const_cast<IDefinitionManager&>(definitionManager), fieldIntrospection, field);
        }
        else if (memberMetaInfo->IsPointer())
        {
            String pointerValue(64, 0);
            sprintf(&pointerValue[0], "[%p] Pointer", field);
            return pointerValue;
        }
        else if (memberInsp->Collection())
        {
            DVASSERT(field != nullptr);
            const InspColl* collection = memberInsp->Collection();
            return Collection(std::make_shared<CollectionImpl>(field, collection));
        }
        else if (memberInsp->Dynamic())
        {
            return Variant();
        }

        return VariantConverter::Instance().Convert(memberInsp->Value(object));
    }

    return Variant();
}

bool DavaMemberProperty::set(const ObjectHandle& pBase, const Variant& v, const IDefinitionManager& definitionManager) const
{
    void* object = UpCast(pBase, definitionManager);
    if (object == nullptr)
        return false;

    const MetaInfo* type = memberInsp->Type();
    if (type->IsPointer() || ((memberInsp->Flags() & I_EDIT) == 0))
        return false;

    VariantType value = VariantConverter::Instance().Convert(v, type);
    memberInsp->SetValue(object, value);
    return true;
}

const MetaBase* DavaMemberProperty::getMetaData() const
{
    return &metaBase;
}

void* DavaMemberProperty::UpCast(ObjectHandle const& pBase, const IDefinitionManager& definitionManager) const
{
    TypeId srcID = pBase.type();
    TypeId dstID(objectType->GetTypeName());
    return reflectedCast(pBase.data(), srcID, dstID, definitionManager);
}

DavaTypeObjectDefinition::DavaTypeObjectDefinition(const InspInfo* objectInsp_)
    : objectInsp(objectInsp_)
{
    DVASSERT(objectInsp != nullptr);
    SetDisplayName(objectInsp->Name().c_str());
}

void DavaTypeObjectDefinition::init(IClassDefinitionModifier& collection)
{
    initModifiers(collection, objectInsp);
}

bool DavaTypeObjectDefinition::isAbstract() const
{
    return false;
}

const char* DavaTypeObjectDefinition::getName() const
{
    return objectInsp->Type()->GetTypeName();
}

const char* DavaTypeObjectDefinition::getParentName() const
{
    DVASSERT(objectInsp != nullptr);
    const InspInfo* parentInsp = objectInsp->BaseInfo();
    return parentInsp == nullptr ? nullptr : parentInsp->Type()->GetTypeName();
}

ObjectHandle DavaTypeObjectDefinition::createBaseProvider(const IClassDefinition& definition, const void* pThis) const
{
    std::shared_ptr<IObjectHandleStorage> storage(new DavaObjectStorage(const_cast<void*>(pThis), objectInsp->Type(),
                                                                        &definition));
    return ObjectHandle(storage);
}

ObjectHandle DavaTypeObjectDefinition::create(const IClassDefinition& definition) const
{
    /*auto pInst = std::unique_ptr<T>(CreateHelper<T>::create());
    return ObjectHandle(std::move(pInst), &definition);*/
    return ObjectHandle();
}

void* DavaTypeObjectDefinition::upCast(void* object) const
{
    if (objectInsp->BaseInfo() != nullptr)
        return object;

    return nullptr;
}

void RegisterDavaType(IDefinitionManager& mng, const InspInfo* inspInfo)
{
    if (inspInfo == nullptr)
        return;
    using TDefinitionMap = std::unordered_map<const MetaInfo*, DavaTypeDefinition*>;
    static TDefinitionMap definitionMap;

    const MetaInfo* type = inspInfo->Type();

    TDefinitionMap::iterator definitionIter = definitionMap.find(type);
    if (definitionIter == definitionMap.end())
    {
        definitionIter = definitionMap.emplace(type, new DavaTypeObjectDefinition(inspInfo)).first;
        mng.registerDefinition(definitionIter->second);
        RegisterDavaType(mng, inspInfo->BaseInfo());
    }
}
}