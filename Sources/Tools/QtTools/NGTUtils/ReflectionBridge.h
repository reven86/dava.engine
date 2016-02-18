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
class NGTMemberProperty : public BaseProperty
{
public:
    NGTMemberProperty(const InspMember* member, const MetaInfo* objectType_);

    bool readOnly() const override;
    bool isValue() const override;
    Variant get(const ObjectHandle& pBase, const IDefinitionManager& definitionManager) const override;
    bool set(const ObjectHandle& pBase, const Variant& v, const IDefinitionManager& definitionManager) const override;
    MetaHandle getMetaData() const override;

private:
    void* UpCast(ObjectHandle const& pBase, const IDefinitionManager& definitionManager) const;

private:
    const MetaInfo* objectType;
    const InspMember* memberInsp;
    MetaHandle metaBase;
    WideString dysplayName;
};

class NGTTypeDefinition : public IClassDefinitionDetails
{
public:
    NGTTypeDefinition(const InspInfo* info);

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
    const InspInfo* info;
    PropertyStorage properties;
    MetaHandle metaHandle;
    WideString displayName;
};

/// Use it only for registration in IDefinitionManager
void RegisterType(IDefinitionManager& mng, const InspInfo* inspInfo);
ObjectHandle CreateObjectHandle(IDefinitionManager& defMng, const InspInfo* fieldInsp, void* field);
}

#endif // DAVAPLUGINTEMPLATE_REFLECTIONBRIDGE_H