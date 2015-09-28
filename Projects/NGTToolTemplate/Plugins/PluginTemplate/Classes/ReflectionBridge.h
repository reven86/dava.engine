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

#include <functional>

namespace DAVA
{
class VariantConverter
{
public:
    VariantType Convert(Variant const& v, MetaInfo const* info) const;
    Variant Convert(VariantType const& value) const;

    static VariantConverter const& Instance();

private:
    VariantConverter();

    using TVtoDV = std::function<VariantType(Variant const& v)>;
    using TDVtoV = std::function<Variant(VariantType const& v)>;
    struct ConvertNode
    {
        TVtoDV vToDvFn;
        TDVtoV dvToVFn;
    };

    Array<ConvertNode, VariantType::TYPES_COUNT> convertFunctions;
};

class DavaMemberProperty : public BaseProperty
{
public:
    // здесь надо сихронизироватся в NGT. Для них TypeId == typeid( T ).name() и для правильного выборе компонента
    // в PropertyTree полагаю нам надо возвращать тоже самое. пока, что я беру Type()->GetTypeName(), а дальше посмотрим
    DavaMemberProperty(const InspMember* member, const MetaInfo* objectType_);

    Variant get(const ObjectHandle& pBase, const IDefinitionManager& definitionManager) const override;
    bool set(const ObjectHandle& pBase, const Variant& v, const IDefinitionManager& definitionManager) const override;
    const MetaBase* getMetaData() const override;

private:
    void* UpCast(ObjectHandle const& pBase, const IDefinitionManager& definitionManager) const;

private:
    const MetaInfo* objectType;
    const InspMember* memberInsp;
    MetaBase& metaBase;
    WideString dysplayName;
};

class DavaTypeDefinition : public IClassDefinitionDetails
{
public:
    DavaTypeDefinition();

    bool isGeneric() const override;
    const MetaBase* getMetaData() const override;
    ObjectHandle createBaseProvider(const ReflectedPolyStruct& polyStruct) const override;
    CastHelperCache* getCastHelperCache() const override;

protected:
    void SetDisplayName(const char* name);

    void initModifiers(IClassDefinitionModifier& collection, const InspInfo* info);

private:
    std::unique_ptr<const MetaBase> metaData;
    mutable CastHelperCache castHelper;
    WideString displayName;
};

class DavaTypeObjectDefinition : public DavaTypeDefinition
{
public:
    DavaTypeObjectDefinition(const InspInfo* objectInsp_);

    void init(IClassDefinitionModifier& collection) override;
    bool isAbstract() const override;
    const char* getName() const override;
    const char* getParentName() const override;

    ObjectHandle createBaseProvider(const IClassDefinition& definition, const void* pThis) const;
    ObjectHandle create(const IClassDefinition& definition) const override;
    void* upCast(void* object) const override;

private:
    InspInfo const* objectInsp;
};

/// Use it only for registration in IDefinitionManager
void RegisterDavaType(IDefinitionManager& mng, const InspInfo* inspInfo);
}

#endif // DAVAPLUGINTEMPLATE_REFLECTIONBRIDGE_H