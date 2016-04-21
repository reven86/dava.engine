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

#include "Extensions.h"

#include "Scene3D/Entity.h"
#include "Render/Material/NMaterial.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"

#include <core_data_model/reflection/reflected_collection_item.hpp>
#include <core_reflection/i_definition_manager.hpp>
#include <core_reflection/metadata/meta_types.hpp>
#include <core_reflection/metadata/meta_impl.hpp>
#include <core_reflection/metadata/meta_utilities.hpp>
#include <core_reflection/base_property.hpp>
#include <core_variant/collection.hpp>

class ProxyProperty : public BaseProperty
{
public:
    ProxyProperty(const char* name)
        : BaseProperty(name, TypeId::getType<ObjectHandle>())
    {
    }

    Variant get(const ObjectHandle& handle, const IDefinitionManager&) const override
    {
        return handle;
    }
};

std::string BuildCollectionElementName(const Collection::ConstIterator& iter, IDefinitionManager& defMng)
{
    Variant v = iter.value();
    ObjectHandle handle;
    if (v.tryCast(handle))
    {
        const IClassDefinition* definition = handle.getDefinition(defMng);
        DVASSERT(definition);

        const MetaDisplayNameObj* displayData = findFirstMetaData<MetaDisplayNameObj>(*definition, defMng);
        return DAVA::WStringToString(DAVA::WideString(displayData->getDisplayName()));
    }
    else
    {
        return iter.key().cast<std::string>();
    }
}

void EntityChildCreatorExtension::exposeChildren(const PropertyNode& node, std::vector<const PropertyNode*>& children, IDefinitionManager& defMng) const
{
#define DEBUG_PROPERTIES_MODEL
    static TypeId entityType = TypeId::getType<DAVA::Entity>();
    static TypeId objectHandleType = TypeId::getType<ObjectHandle>();
    static TypeId nmaterialType = TypeId::getType<DAVA::NMaterial*>();

#if defined(DEBUG_PROPERTIES_MODEL)
    std::string propertyName = node.propertyInstance->getName();
#endif
    if (node.propertyType == DAVAProperiesEnum::EntityRoot)
    {
        PropertyNode newNode = node;
        newNode.propertyType = PropertyNode::RealProperty;
        ChildCreatorExtension::exposeChildren(newNode, children, defMng);

        for (auto iter = children.rbegin(); iter != children.rend(); ++iter)
        {
            const PropertyNode* child = *iter;
            if (strcmp(child->propertyInstance->getName(), "components") == 0)
            {
                auto forwardIter = (iter + 1).base();
                allocator->deletePropertyNode(*forwardIter);
                children.erase(forwardIter);
                break;
            }
        }
    }
    else if (node.object.type() == entityType && node.propertyType == PropertyNode::SelfRoot)
    {
        static IBasePropertyPtr entityProxy = std::make_shared<ProxyProperty>("Entity");
        children.push_back(allocator->createPropertyNode(entityProxy, node.object, DAVAProperiesEnum::EntityRoot));
        DAVA::Entity* entity = node.object.getBase<DAVA::Entity>();
        DVASSERT(entity != nullptr);

        const IClassDefinition* definition = node.object.getDefinition(defMng);
        DVASSERT(definition != nullptr);
        IBasePropertyPtr components = definition->findProperty("components");
        DVASSERT(components != nullptr);
        Variant componentsCollection = components->get(node.object, defMng);

        Collection collection;
        if (componentsCollection.tryCast(collection))
        {
            for (auto iter = collection.begin(); iter != collection.end(); ++iter)
            {
                ReflectedIteratorValue value;
                value.iterator = iter;
                value.value = iter.value();

                std::string name = BuildCollectionElementName(iter, defMng);
                IBasePropertyPtr property = allocator->getCollectionItemProperty(std::move(name), iter.value().type()->typeId(), defMng);
                children.push_back(allocator->createPropertyNode(property, ObjectHandle(value), PropertyNode::CollectionItem));
            }
        }
    }
    else if (node.propertyInstance->getType() == nmaterialType)
    {
        // do not expose material children in property panel
        return;
    }
    else
    {
        ChildCreatorExtension::exposeChildren(node, children, defMng);
    }
}

RefPropertyItem * EntityMergeValueExtension::lookUpItem(const PropertyNode * node, const std::vector<std::unique_ptr<RefPropertyItem>>& items,
                                                        IDefinitionManager & definitionManager) const
{
    return MergeValuesExtension::lookUpItem(node, items, definitionManager);
}
