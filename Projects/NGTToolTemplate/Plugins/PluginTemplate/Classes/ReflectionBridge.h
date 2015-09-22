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

#ifndef DAVAPLUGINTEMPLATE_REFLECTIONBRIDGE_H
#define DAVAPLUGINTEMPLATE_REFLECTIONBRIDGE_H

#include "core_reflection/interfaces/i_class_definition.hpp"
#include "core_reflection/reflected_object.hpp"
#include "core_reflection/metadata/meta_types.hpp"
#include "core_reflection/base_property.hpp"
#include "core_reflection/type_class_definition.hpp"

#include "core_variant/variant.hpp"

#include "Base/BaseTypes.h"
#include "Base/Introspection.h"
#include "Base/IntrospectionBase.h"

class VariantConverter
{
public:
    VariantConverter()
    {
        typeMapping.fill(nullptr);
        IMetaTypeManager* metaTypeMng = Variant::getMetaTypeManager();
        typeMapping[DAVA::VariantType::TYPE_NONE] = metaTypeMng->findType<void>();
        typeMapping[DAVA::VariantType::TYPE_INT32] = metaTypeMng->findType<DAVA::int64>();
        typeMapping[DAVA::VariantType::TYPE_FLOAT] = metaTypeMng->findType<DAVA::float32>();
        typeMapping[DAVA::VariantType::TYPE_STRING] = metaTypeMng->findType<DAVA::String>();
        typeMapping[DAVA::VariantType::TYPE_WIDE_STRING] = metaTypeMng->findType<DAVA::WideString>();
        typeMapping[DAVA::VariantType::TYPE_UINT32] = metaTypeMng->findType<DAVA::uint64>();
        typeMapping[DAVA::VariantType::TYPE_INT64] = metaTypeMng->findType<DAVA::int64>();
        typeMapping[DAVA::VariantType::TYPE_UINT64] = metaTypeMng->findType<DAVA::uint64>();
        typeMapping[DAVA::VariantType::TYPE_VECTOR2] = metaTypeMng->findType<DAVA::Vector2>();
        typeMapping[DAVA::VariantType::TYPE_VECTOR3] = metaTypeMng->findType<DAVA::Vector3>();
        typeMapping[DAVA::VariantType::TYPE_VECTOR4] = metaTypeMng->findType<DAVA::Vector4>();
        typeMapping[DAVA::VariantType::TYPE_MATRIX2] = metaTypeMng->findType<DAVA::Matrix2>();
        typeMapping[DAVA::VariantType::TYPE_MATRIX3] = metaTypeMng->findType<DAVA::Matrix3>();
        typeMapping[DAVA::VariantType::TYPE_MATRIX4] = metaTypeMng->findType<DAVA::Matrix4>();
        typeMapping[DAVA::VariantType::TYPE_COLOR] = metaTypeMng->findType<DAVA::Color>();
        typeMapping[DAVA::VariantType::TYPE_FASTNAME] = metaTypeMng->findType<DAVA::FastName>();
        typeMapping[DAVA::VariantType::TYPE_AABBOX3] = metaTypeMng->findType<DAVA::AABBox3>();
        typeMapping[DAVA::VariantType::TYPE_FILEPATH] = metaTypeMng->findType<DAVA::FilePath>();

#ifdef __DAVAENGINE_DEBUG__
/*for (size_t i = 0; i < typeMapping.size(); ++i)
        {
            DAVA::VariantType::eVariantType t = static_cast<DAVA::VariantType::eVariantType>(i);
            if (t != DAVA::VariantType::TYPE_KEYED_ARCHIVE && t != DAVA::VariantType::TYPE_BYTE_ARRAY)
                assert(typeMapping[i] != nullptr);
        }*/
#endif
    }

    DAVA::VariantType Convert(Variant const& v)
    {
        DAVA::VariantType::eVariantType davaType = DAVA::VariantType::TYPE_NONE;
        const MetaType* valueType = v.type();
        for (size_t i = 0; i < typeMapping.size(); ++i)
        {
            if (typeMapping[i] == valueType)
            {
                davaType = static_cast<DAVA::VariantType::eVariantType>(i);
                break;
            }
        }

        switch (davaType)
        {
        case DAVA::VariantType::TYPE_BOOLEAN:
            return DAVA::VariantType(v.value<bool>());
        case DAVA::VariantType::TYPE_INT32:
            return DAVA::VariantType(v.value<DAVA::int32>());
        case DAVA::VariantType::TYPE_FLOAT:
            return DAVA::VariantType(v.value<DAVA::float32>());
        case DAVA::VariantType::TYPE_STRING:
            return DAVA::VariantType(v.value<DAVA::String>());
        case DAVA::VariantType::TYPE_WIDE_STRING:
            return DAVA::VariantType(v.value<DAVA::WideString>());
        case DAVA::VariantType::TYPE_UINT32:
            return DAVA::VariantType(v.value<DAVA::uint32>());
        case DAVA::VariantType::TYPE_INT64:
            return DAVA::VariantType(v.value<DAVA::int64>());
        case DAVA::VariantType::TYPE_UINT64:
            return DAVA::VariantType(v.value<DAVA::uint64>());
        case DAVA::VariantType::TYPE_VECTOR2:
            return DAVA::VariantType(v.value<DAVA::Vector2>());
        case DAVA::VariantType::TYPE_VECTOR3:
            return DAVA::VariantType(v.value<DAVA::Vector3>());
        case DAVA::VariantType::TYPE_VECTOR4:
            return DAVA::VariantType(v.value<DAVA::Vector4>());
        case DAVA::VariantType::TYPE_MATRIX2:
            return DAVA::VariantType(v.value<DAVA::Matrix2>());
        case DAVA::VariantType::TYPE_MATRIX3:
            return DAVA::VariantType(v.value<DAVA::Matrix3>());
        case DAVA::VariantType::TYPE_MATRIX4:
            return DAVA::VariantType(v.value<DAVA::Matrix4>());
        /*case DAVA::VariantType::TYPE_COLOR:         return DAVA::VariantType(v.value<DAVA::Color>());
            case DAVA::VariantType::TYPE_FASTNAME:      return DAVA::VariantType(v.value<DAVA::FastName>());
            case DAVA::VariantType::TYPE_AABBOX3:       return DAVA::VariantType(v.value<DAVA::AABBox3>());
            case DAVA::VariantType::TYPE_FILEPATH:      return DAVA::VariantType(v.value<DAVA::FilePath>());*/
        case DAVA::VariantType::TYPE_NONE:
            break;
        default:
            DVASSERT(false);
        }

        return DAVA::VariantType();
    }

    Variant Convert(DAVA::VariantType const& value)
    {
        switch (value.GetType())
        {
        case DAVA::VariantType::TYPE_BOOLEAN:
            return Variant(value.AsBool());
        case DAVA::VariantType::TYPE_INT32:
            return Variant(value.AsInt32());
        case DAVA::VariantType::TYPE_FLOAT:
            return Variant(value.AsFloat());
        case DAVA::VariantType::TYPE_STRING:
            return Variant(value.AsString());
        case DAVA::VariantType::TYPE_WIDE_STRING:
            return Variant(value.AsWideString());
        case DAVA::VariantType::TYPE_UINT32:
            return Variant(value.AsUInt32());
        case DAVA::VariantType::TYPE_INT64:
            return Variant(value.AsInt64());
        case DAVA::VariantType::TYPE_UINT64:
            return Variant(value.AsUInt64());
        case DAVA::VariantType::TYPE_VECTOR2:
            return Variant(value.AsVector2());
        case DAVA::VariantType::TYPE_VECTOR3:
            return Variant(value.AsVector3());
        case DAVA::VariantType::TYPE_VECTOR4:
            return Variant(value.AsVector4());
        case DAVA::VariantType::TYPE_MATRIX2:
            return Variant(value.AsMatrix2());
        case DAVA::VariantType::TYPE_MATRIX3:
            return Variant(value.AsMatrix3());
        case DAVA::VariantType::TYPE_MATRIX4:
            return Variant(value.AsMatrix4());
        /*case DAVA::VariantType::TYPE_COLOR:           return Variant(value.AsColor());
        case DAVA::VariantType::TYPE_FASTNAME:        return Variant(value.AsFastName());
        case DAVA::VariantType::TYPE_AABBOX3:         return Variant(value.AsAABBox3());
        case DAVA::VariantType::TYPE_FILEPATH:        return Variant(value.AsFilePath());*/
        default:
            DVASSERT(false);
            break;
        }

        return Variant();
    }

    static VariantConverter& Instance()
    {
        static VariantConverter c;
        return c;
    }

private:
    DAVA::Array<const MetaType*, DAVA::VariantType::TYPES_COUNT> typeMapping;
};

template <typename TObject>
class DavaMemberProperty : public BaseProperty
{
public:
    // здесь надо сихронизироватся в NGT. Для них TypeId == typeid( T ).name() и для правильного выборе компонента
    // в PropertyTree полагаю нам надо возвращать тоже самое. пока, что я беру Type()->GetTypeName(), а дальше посмотрим
    DavaMemberProperty(const DAVA::InspMember* member)
        : BaseProperty(member->Name().c_str(), member->Type()->GetTypeName())
        , memberInsp(member)
    {
    }

    Variant get(const ObjectHandle& pBase, const IDefinitionManager& definitionManager) const override
    {
        TObject* object = reflectedCast<TObject>(pBase, definitionManager).get();
        if (object)
            return VariantConverter::Instance().Convert(memberInsp->Value(object));

        return Variant();
    }

    //==========================================================================
    bool set(const ObjectHandle& pBase, const Variant& v, const IDefinitionManager& definitionManager) const override
    {
        TObject* object = reflectedCast<TObject>(pBase, definitionManager).get();
        if (object == nullptr)
            return false;

        DAVA::VariantType value = VariantConverter::Instance().Convert(v);
        memberInsp->SetValue(object, value);
        return true;
    }

private:
    const DAVA::InspMember* memberInsp;
};

#include "Base/IntrospectionBase.h"

template <typename T>
class DavaTypeClassDefinition : public IClassDefinitionDetails
{
public:
    DavaTypeClassDefinition()
        : metaData(&(MetaNone()))
    {
    }

    void init(IClassDefinitionModifier& collection) override
    {
        initModifiers(collection, DAVA::GetIntrospection<T>());
    }

    bool isAbstract() const override
    {
        return std::is_abstract<T>();
    }

    virtual bool isGeneric() const override
    {
        return false;
    }

    const char* getName() const override
    {
        return getClassIdentifier<T>();
    }

    const char* getParentName() const override
    {
        return nullptr;
    }

    const MetaBase* getMetaData() const override
    {
        return metaData.get();
    }

    ObjectHandle createBaseProvider(const ReflectedPolyStruct&) const override
    {
        DVASSERT_MSG(false, ("Dava objects don't derived from ReflectedPolyStruct"));
        return ObjectHandle();
    }

    ObjectHandle createBaseProvider(const IClassDefinition& definition, const void* pThis) const
    {
        return ObjectHandle(static_cast<const T*>(pThis), &definition);
    }

    ObjectHandle create(const IClassDefinition& definition) const override
    {
        auto pInst = std::unique_ptr<T>(CreateHelper<T>::create());
        return ObjectHandle(std::move(pInst), &definition);
    }

    CastHelperCache* getCastHelperCache() const override
    {
        return nullptr;
    }

    void* upCast(void* object) const override
    {
        return nullptr;
    }

private:
    void initModifiers(IClassDefinitionModifier& collection, const DAVA::InspInfo* info)
    {
        if (info == nullptr)
            return;

        for (int i = 0; i < info->MembersCount(); ++i)
        {
            const DAVA::InspMember* member = info->Member(i);

            //void * memberObject = member->Data();
            //InspInfo const * memberInsp = metaInfo->GetIntrospection()

            DAVA::MetaInfo const* metaInfo = member->Type();
            int memberFlags = member->Flags();

            //if (есть интроспекция у поля и это интроспекция KeyedArchive)
            //else if (есть указатель на memberObject и есть инстроспекция)
            //else
            //{
            //if (metaInfo->IsPointer())
            //  create read-only property
            //else if (member->Collection() != nullptr)
            // create collection property
            //else if (member->Dynamic() != nullptr)
            // create dynamic member property
            //else
            collection.addProperty(new DavaMemberProperty<T>(member), &(MetaNone()));
            //}

            //collection.addProperty(FunctionPropertyHelper<T>::getBaseProperty(member->Name().c_str(), ))
        }

        initModifiers(collection, info->BaseInfo());
    }

private:
    std::unique_ptr<const MetaBase> metaData;
};

template <typename T>
typename std::enable_if<DAVA::HasInsp<T>::result, DavaTypeClassDefinition<T>*>::type CreateDavaClassDefinition()
{
    return new DavaTypeClassDefinition<T>();
}

#endif // DAVAPLUGINTEMPLATE_REFLECTIONBRIDGE_H