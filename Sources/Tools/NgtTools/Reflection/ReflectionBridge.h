#ifndef __QTTOOLS_REFLECTIONBRIDGE_H__
#define __QTTOOLS_REFLECTIONBRIDGE_H__

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

namespace NGTLayer
{
class NGTMemberProperty : public BaseProperty
{
public:
    NGTMemberProperty(const DAVA::InspMember* member, const DAVA::MetaInfo* objectType);

    bool readOnly() const override;
    bool isValue() const override;
    Variant get(const ObjectHandle& pBase, const IDefinitionManager& definitionManager) const override;
    bool set(const ObjectHandle& pBase, const Variant& v, const IDefinitionManager& definitionManager) const override;
    MetaHandle getMetaData() const override;

private:
    void* UpCast(ObjectHandle const& pBase, const IDefinitionManager& definitionManager) const;

private:
    const DAVA::MetaInfo* objectType;
    const DAVA::InspMember* memberInsp;
    MetaHandle metaBase;
    DAVA::WideString dysplayName;
};

class NGTTypeDefinition : public IClassDefinitionDetails
{
public:
    NGTTypeDefinition(const DAVA::InspInfo* info);

    bool isAbstract() const override;
    bool isGeneric() const override;

    const char* getName() const override;
    const char* getParentName() const override;
    ObjectHandle create(const IClassDefinition& classDefinition) const override;
    void* upCast(void* object) const override;
    PropertyIteratorImplPtr getPropertyIterator() const override;
    IClassDefinitionModifier* getDefinitionModifier() const override;

    MetaHandle getMetaData() const override;

private:
    const DAVA::InspInfo* info;
    PropertyStorage properties;
    MetaHandle metaHandle;
    DAVA::WideString displayName;
};

/// Use it only for registration in IDefinitionManager
void RegisterType(IDefinitionManager& mng, const DAVA::InspInfo* inspInfo);
ObjectHandle CreateObjectHandle(IDefinitionManager& defMng, const DAVA::InspInfo* fieldInsp, void* field);
} // namespace NGTLayer

#endif // __QTTOOLS_REFLECTIONBRIDGE_H__