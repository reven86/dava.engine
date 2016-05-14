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

#include "NgtTools/Common/GlobalContext.h"
#include "Scene3D/Entity.h"
#include "Render/Material/NMaterial.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Render/Highlevel/RenderBatch.h"

#include <core_qt_common/models/buttons_model.hpp>
#include <core_data_model/i_item_role.hpp>
#include <core_data_model/reflection/reflected_collection_item.hpp>
#include <core_data_model/reflection/reflected_property_model.hpp>
#include <core_reflection/i_definition_manager.hpp>
#include <core_reflection/metadata/meta_types.hpp>
#include <core_reflection/metadata/meta_impl.hpp>
#include <core_reflection/metadata/meta_utilities.hpp>
#include <core_reflection/base_property.hpp>
#include <core_reflection/object_handle.hpp>
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

namespace ExtensionsDetails
{
const TypeId entityType = TypeId::getType<DAVA::Entity>();
const TypeId objectHandleType = TypeId::getType<ObjectHandle>();
const TypeId nmaterialType = TypeId::getType<DAVA::NMaterial*>();
const TypeId keyedArchive = TypeId::getType<DAVA::KeyedArchive*>();
const TypeId renderBatchType = TypeId::getType<DAVA::RenderBatch>();
}

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
    else if (node.object.type() == ExtensionsDetails::entityType && node.propertyType == PropertyNode::SelfRoot)
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
    else if (node.propertyInstance->getType() == ExtensionsDetails::nmaterialType)
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

Variant PropertyPanelGetExtension::getValue(const RefPropertyItem* item, int column, size_t roleId, IDefinitionManager & definitionManager) const
{
    if (column == 1 && roleId == ValueRole::roleId_)
    {
        Variant result = GetterExtension::getValue(item, column, roleId, definitionManager);
        ObjectHandle handle;
        if (result.tryCast(handle))
        {
            TypeId type = handle.type();
            if (type.isPointer())
            {
                type = type.removePointer();
            }

            return type.getName();
        }
    }
    return GetterExtension::getValue(item, column, roleId, definitionManager);
}

EntityInjectDataExtension::EntityInjectDataExtension(Delegate & delegateObj_, IDefinitionManager& defManager_)
    : delegateObj(delegateObj_)
    , defManager(defManager_)
{
}

void EntityInjectDataExtension::inject(const RefPropertyItem* item, const std::function<void(size_t, const Variant&)>& injector)
{
    static TypeId removableComponents[] = { TypeId::getType<DAVA::RenderComponent>(), TypeId::getType<DAVA::ActionComponent>() };

    const PropertyNode* node = item->getObjects().front();
    Variant value = node->propertyInstance->get(node->object, defManager);
    ObjectHandle handle;
    if (value.tryCast(handle))
    {
        TypeId type = handle.type();
        if (type.isPointer())
            type = type.removePointer();

        if (std::find(std::begin(removableComponents), std::end(removableComponents), type) != std::end(removableComponents))
        {
            std::vector<ButtonItem> buttons;
            buttons.emplace_back(true, "/QtIcons/remove.png",std::bind(&EntityInjectDataExtension::RemoveComponent, this, item));
            ButtonsModel* buttonsModel = new ButtonsModel(std::move(buttons));
            injector(ButtonsDefinitionRole::roleId_, Variant(ObjectHandle(std::unique_ptr<IListModel>(buttonsModel))));
        }
        else if (type == ExtensionsDetails::renderBatchType)
        {
            std::vector<ButtonItem> buttons;
            buttons.emplace_back(true, "/QtIcons/external.png", std::bind(&EntityInjectDataExtension::AddCustomProperty, this, item));
            buttons.emplace_back(true, "/QtIcons/shadow.png", std::bind(&EntityInjectDataExtension::AddCustomProperty, this, item));
            buttons.emplace_back(true, "/QtIcons/remove.png", std::bind(&EntityInjectDataExtension::AddCustomProperty, this, item));
            ButtonsModel* buttonsModel = new ButtonsModel(std::move(buttons));
            injector(ButtonsDefinitionRole::roleId_, Variant(ObjectHandle(std::unique_ptr<IListModel>(buttonsModel))));
        }
        else if (node->propertyInstance->getType() == ExtensionsDetails::nmaterialType)
        {
            std::vector<ButtonItem> buttons;
            buttons.emplace_back(true, "/QtIcons/3d.png", std::bind(&EntityInjectDataExtension::OpenMaterials, this, item));
            ButtonsModel* buttonsModel = new ButtonsModel(std::move(buttons));
            injector(ButtonsDefinitionRole::roleId_, Variant(ObjectHandle(std::unique_ptr<IListModel>(buttonsModel))));
        }
    }
    else if (node->propertyInstance->getType() == ExtensionsDetails::keyedArchive)
    {
        std::vector<ButtonItem> buttons;
        buttons.emplace_back(true, "/QtIcons/keyplus.png", std::bind(&EntityInjectDataExtension::AddCustomProperty, this, item));
        ButtonsModel* buttonsModel = new ButtonsModel(std::move(buttons));
        injector(ButtonsDefinitionRole::roleId_, Variant(ObjectHandle(std::unique_ptr<IListModel>(buttonsModel))));
    }
}

void EntityInjectDataExtension::RemoveComponent(const RefPropertyItem* item)
{
    const std::vector<const PropertyNode*>& objects = item->getObjects();
    delegateObj.StartBatch("Remove component", objects.size());
    for (const PropertyNode* object : objects)
    {
        Variant value = object->propertyInstance->get(object->object, defManager);
        ObjectHandle handle;
        DVVERIFY(value.tryCast(handle));

        DAVA::Component* component = reflectedCast<DAVA::Component>(handle.data(), handle.type(), defManager);
        DVASSERT(component != nullptr);

        delegateObj.RemoveComponent(component);
    }
    delegateObj.EndBatch();
}

void EntityInjectDataExtension::OpenMaterials(const RefPropertyItem* item)
{
    const PropertyNode* node = item->getObjects().front();
    Variant value = node->propertyInstance->get(node->object, defManager);
    ObjectHandle handle;
    DVVERIFY(value.tryCast(handle));

    DAVA::NMaterial * material = reflectedCast<DAVA::NMaterial>(handle.data(), handle.type(), defManager);
    DVASSERT(material != nullptr);
    delegateObj.OpenMaterial(material);
}

void EntityInjectDataExtension::AddCustomProperty(const RefPropertyItem* item)
{

}
