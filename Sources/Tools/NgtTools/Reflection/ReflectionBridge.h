#pragma once

#include <core_reflection/interfaces/i_class_definition.hpp>
#include <core_reflection/reflected_object.hpp>
#include <core_reflection/metadata/meta_types.hpp>
#include <core_reflection/base_property.hpp>
#include <core_reflection/type_class_definition.hpp>

#include <core_variant/variant.hpp>

#include "Base/BaseTypes.h"
#include "Base/Introspection.h"
#include "Base/IntrospectionBase.h"

#include <functional>

namespace NGTLayer
{
class NGTMemberProperty : public wgt::BaseProperty
{
public:
    NGTMemberProperty(const DAVA::InspMember* member, const DAVA::MetaInfo* objectType);

    bool readOnly() const override;
    bool isValue() const override;
    wgt::Variant get(const wgt::ObjectHandle& pBase, const wgt::IDefinitionManager& definitionManager) const override;
    bool set(const wgt::ObjectHandle& pBase, const wgt::Variant& v, const wgt::IDefinitionManager& definitionManager) const override;
    wgt::MetaHandle getMetaData() const override;

private:
    void* UpCast(wgt::ObjectHandle const& pBase, const wgt::IDefinitionManager& definitionManager) const;

private:
    const DAVA::MetaInfo* objectType;
    const DAVA::InspMember* memberInsp;
    wgt::MetaHandle metaBase;
    DAVA::WideString dysplayName;
};

class NGTTypeDefinition : public wgt::IClassDefinitionDetails
{
public:
    NGTTypeDefinition(const DAVA::InspInfo* info);

    bool isAbstract() const override;
    bool isGeneric() const override;

    const char* getName() const override;
    const char* getParentName() const override;
    wgt::ObjectHandle create(const wgt::IClassDefinition& classDefinition) const override;
    void* upCast(void* object) const override;
    wgt::PropertyIteratorImplPtr getPropertyIterator() const override;
    wgt::IClassDefinitionModifier* getDefinitionModifier() const override;

    wgt::MetaHandle getMetaData() const override;

private:
    const DAVA::InspInfo* info;
    wgt::PropertyStorage properties;
    wgt::MetaHandle metaHandle;
    DAVA::WideString displayName;
};

/// Use it only for registration in IDefinitionManager
void RegisterType(wgt::IDefinitionManager& mng, const DAVA::InspInfo* inspInfo);
wgt::ObjectHandle CreateObjectHandle(wgt::IDefinitionManager& defMng, const DAVA::InspInfo* fieldInsp, void* field);
} // namespace NGTLayer