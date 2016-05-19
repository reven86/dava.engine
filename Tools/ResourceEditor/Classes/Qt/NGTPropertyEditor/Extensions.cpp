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

#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Deprecated/EditorConfig.h"

#include "NgtTools/Reflection/NGTCollectionsImpl.h"
#include "NgtTools/Common/GlobalContext.h"

#include "Commands2/Actions/ShowMaterialAction.h"
#include "Commands2/ConvertToShadowCommand.h"
#include "Commands2/KeyedArchiveCommand.h"
#include "Commands2/DeleteRenderBatchCommand.h"
#include "Commands2/RebuildTangentSpaceCommand.h"
#include "Commands2/RemoveComponentCommand.h"

#include "Scene3D/Entity.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Material/NMaterial.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"

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

#include <QMessageBox>

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

DAVA::RenderBatch* ExtractRenderBatch(const RefPropertyItem* item, IDefinitionManager& defManager)
{
    const std::vector<std::shared_ptr<const PropertyNode>>& objects = item->getObjects();
    DVASSERT(objects.size() == 1);

    std::shared_ptr<const PropertyNode> node = objects.front();
    ObjectHandle valueHandle;
    DVVERIFY(node->propertyInstance->get(node->object, defManager).tryCast(valueHandle));
    DAVA::RenderBatch* batch = valueHandle.getBase<DAVA::RenderBatch>();
    DVASSERT(batch != nullptr);

    return batch;
}

DAVA::Entity* FindEntityWithRenderObject(const RefPropertyItem* item, DAVA::RenderObject* renderObject)
{
    const RefPropertyItem* parent = item->getParent();
    while (parent != nullptr)
    {
        item = parent;
        parent = item->getParent();
    }

    const std::vector<std::shared_ptr<const PropertyNode>>& objects = item->getObjects();
    DVASSERT(objects.size() == 1);
    std::shared_ptr<const PropertyNode> object = objects.front();
    DAVA::Entity* entity = object->object.getBase<DAVA::Entity>();

    DVASSERT(GetRenderObject(entity) == renderObject);

    return entity;
}
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

void EntityChildCreatorExtension::exposeChildren(const std::shared_ptr<const PropertyNode>& node, std::vector<std::shared_ptr<const PropertyNode>>& children, IDefinitionManager& defMng) const
{
    if (node->propertyType == DAVAProperiesEnum::EntityRoot)
    {
        std::shared_ptr<const PropertyNode> newNode = allocator->createPropertyNode(node->propertyInstance, node->object);
        ChildCreatorExtension::exposeChildren(newNode, children, defMng);

        for (auto iter = children.rbegin(); iter != children.rend(); ++iter)
        {
            const std::shared_ptr<const PropertyNode> child = *iter;
            if (strcmp(child->propertyInstance->getName(), "components") == 0)
            {
                auto forwardIter = (iter + 1).base();
                children.erase(forwardIter);
                break;
            }
        }
    }
    else if (node->object.type() == ExtensionsDetails::entityType && node->propertyType == PropertyNode::SelfRoot)
    {
        static IBasePropertyPtr entityProxy = std::make_shared<ProxyProperty>("Entity");
        children.push_back(allocator->createPropertyNode(entityProxy, node->object, DAVAProperiesEnum::EntityRoot));
        DAVA::Entity* entity = node->object.getBase<DAVA::Entity>();
        DVASSERT(entity != nullptr);

        const IClassDefinition* definition = node->object.getDefinition(defMng);
        DVASSERT(definition != nullptr);
        IBasePropertyPtr components = definition->findProperty("components");
        DVASSERT(components != nullptr);
        Variant componentsCollection = components->get(node->object, defMng);

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
    else if (node->propertyInstance->getType() == ExtensionsDetails::nmaterialType)
    {
        // do not expose material children in property panel
        return;
    }
    else
    {
        ChildCreatorExtension::exposeChildren(node, children, defMng);
    }
}

RefPropertyItem* EntityMergeValueExtension::lookUpItem(const std::shared_ptr<const PropertyNode>& node, const std::vector<std::unique_ptr<RefPropertyItem>>& items,
                                                       IDefinitionManager& definitionManager) const
{
    return MergeValuesExtension::lookUpItem(node, items, definitionManager);
}

Variant PropertyPanelGetExtension::getValue(const RefPropertyItem* item, int column, size_t roleId, IDefinitionManager& definitionManager) const
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

EntityInjectDataExtension::EntityInjectDataExtension(Delegate& delegateObj_, IDefinitionManager& defManager_)
    : delegateObj(delegateObj_)
    , defManager(defManager_)
{
}

void EntityInjectDataExtension::inject(RefPropertyItem* item)
{
    static TypeId removableComponents[] = { TypeId::getType<DAVA::RenderComponent>(), TypeId::getType<DAVA::ActionComponent>() };

    std::shared_ptr<const PropertyNode> node = item->getObjects().front();
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
            buttons.emplace_back(true, "/QtIcons/remove.png", std::bind(&EntityInjectDataExtension::RemoveComponent, this, item));
            ButtonsModel* buttonsModel = new ButtonsModel(std::move(buttons));
            item->injectData(ButtonsDefinitionRole::roleId_, Variant(ObjectHandle(std::unique_ptr<IListModel>(buttonsModel))));
        }
        else if (type == ExtensionsDetails::renderBatchType)
        {
            std::vector<ButtonItem> buttons;
            buttons.emplace_back(true, "/QtIcons/external.png", std::bind(&EntityInjectDataExtension::RebuildTangentSpace, this, item));
            buttons.emplace_back(true, "/QtIcons/shadow.png", std::bind(&EntityInjectDataExtension::ConvertBatchToShadow, this, item));
            buttons.emplace_back(true, "/QtIcons/remove.png", std::bind(&EntityInjectDataExtension::RemoveRenderBatch, this, item));
            ButtonsModel* buttonsModel = new ButtonsModel(std::move(buttons));
            item->injectData(ButtonsDefinitionRole::roleId_, Variant(ObjectHandle(std::unique_ptr<IListModel>(buttonsModel))));
        }
        else if (node->propertyInstance->getType() == ExtensionsDetails::nmaterialType)
        {
            std::vector<ButtonItem> buttons;
            buttons.emplace_back(true, "/QtIcons/3d.png", std::bind(&EntityInjectDataExtension::OpenMaterials, this, item));
            ButtonsModel* buttonsModel = new ButtonsModel(std::move(buttons));
            item->injectData(ButtonsDefinitionRole::roleId_, Variant(ObjectHandle(std::unique_ptr<IListModel>(buttonsModel))));
        }
    }
    else if (node->propertyInstance->getType() == ExtensionsDetails::keyedArchive)
    {
        std::vector<ButtonItem> buttons;
        buttons.emplace_back(true, "/QtIcons/keyplus.png", std::bind(&EntityInjectDataExtension::AddCustomProperty, this, item));
        ButtonsModel* buttonsModel = new ButtonsModel(std::move(buttons));
        item->injectData(ButtonsDefinitionRole::roleId_, Variant(ObjectHandle(std::unique_ptr<IListModel>(buttonsModel))));
    }
}

void EntityInjectDataExtension::updateInjection(RefPropertyItem* item)
{
    Variant buttons = item->getInjectedData(ButtonsDefinitionRole::roleId_);
    ObjectHandle modelHandle;
    if (!buttons.tryCast(modelHandle))
        return;

    ButtonsModel* model = dynamic_cast<ButtonsModel*>(modelHandle.getBase<IListModel>());

    if (model == nullptr)
    {
        return;
    }

    const std::vector<std::shared_ptr<const PropertyNode>>& objects = item->getObjects();
    DVASSERT(!objects.empty());

    bool isSingleSelection = objects.size() == 1;
    std::shared_ptr<const PropertyNode> node = objects.front();
    ObjectHandle valueHandle;

    if (node->propertyInstance->get(node->object, defManager).tryCast(valueHandle))
    {
        TypeId type = valueHandle.type();
        if (type.isPointer())
            type = type.removePointer();

        if (type == ExtensionsDetails::renderBatchType)
        {
            bool canConvertToShadow = false;
            bool isRebuildTsEnabled = false;
            DAVA::RenderBatch* batch = valueHandle.getBase<DAVA::RenderBatch>();
            if (batch != nullptr)
            {
                DAVA::RenderObject* renderObject = batch->GetRenderObject();
                if (renderObject != nullptr)
                {
                    canConvertToShadow = ConvertToShadowCommand::CanConvertBatchToShadow(batch);
                }

                DAVA::PolygonGroup* group = batch->GetPolygonGroup();
                if (group != nullptr)
                {
                    const DAVA::int32 requiredVertexFormat = (DAVA::EVF_TEXCOORD0 | DAVA::EVF_NORMAL);
                    isRebuildTsEnabled = (group->GetPrimitiveType() == rhi::PRIMITIVE_TRIANGLELIST);
                    isRebuildTsEnabled &= ((group->GetFormat() & requiredVertexFormat) == requiredVertexFormat);
                }
            }

            model->setEnabled(0, isSingleSelection && isRebuildTsEnabled);
            model->setEnabled(1, isSingleSelection && canConvertToShadow);
            model->setEnabled(2, isSingleSelection);
        }
        else if (node->propertyInstance->getType() == ExtensionsDetails::nmaterialType)
        {
            // Show material button
            model->setEnabled(0, isSingleSelection);
        }
    }
}

void EntityInjectDataExtension::RemoveComponent(const RefPropertyItem* item)
{
    const std::vector<std::shared_ptr<const PropertyNode>>& objects = item->getObjects();
    delegateObj.StartBatch("Remove component", static_cast<DAVA::uint32>(objects.size()));
    for (const std::shared_ptr<const PropertyNode>& object : objects)
    {
        Variant value = object->propertyInstance->get(object->object, defManager);
        ObjectHandle handle;
        DVVERIFY(value.tryCast(handle));

        DAVA::Component* component = reflectedCast<DAVA::Component>(handle.data(), handle.type(), defManager);
        DVASSERT(component != nullptr);

        delegateObj.Exec(Command2::Create<RemoveComponentCommand>(component->GetEntity(), component));
    }
    delegateObj.EndBatch();
}

void EntityInjectDataExtension::RemoveRenderBatch(const RefPropertyItem* item)
{
    DAVA::RenderBatch* batch = ExtensionsDetails::ExtractRenderBatch(item, defManager);
    DAVA::RenderObject* renderObject = batch->GetRenderObject();
    DVASSERT(renderObject != nullptr);

    DAVA::uint32 batchIndex = static_cast<DAVA::uint32>(-1);
    for (DAVA::uint32 i = 0; i < renderObject->GetRenderBatchCount(); ++i)
    {
        if (renderObject->GetRenderBatch(i) == batch)
        {
            batchIndex = i;
            break;
        }
    }
    DVASSERT(batchIndex != static_cast<DAVA::uint32>(-1));

    DAVA::Entity* entity = ExtensionsDetails::FindEntityWithRenderObject(item, renderObject);
    delegateObj.Exec(Command2::Create<DeleteRenderBatchCommand>(entity, renderObject, batchIndex));
}

void EntityInjectDataExtension::ConvertBatchToShadow(const RefPropertyItem* item)
{
    DAVA::RenderBatch* batch = ExtensionsDetails::ExtractRenderBatch(item, defManager);

    DAVA::Entity* entity = ExtensionsDetails::FindEntityWithRenderObject(item, batch->GetRenderObject());

    delegateObj.Exec(Command2::Create<ConvertToShadowCommand>(entity, batch));
}

void EntityInjectDataExtension::RebuildTangentSpace(const RefPropertyItem* item)
{
    DAVA::RenderBatch* batch = ExtensionsDetails::ExtractRenderBatch(item, defManager);
    delegateObj.Exec(Command2::Create<RebuildTangentSpaceCommand>(batch, true));
}

void EntityInjectDataExtension::OpenMaterials(const RefPropertyItem* item)
{
    std::shared_ptr<const PropertyNode> node = item->getObjects().front();
    Variant value = node->propertyInstance->get(node->object, defManager);
    ObjectHandle handle;
    DVVERIFY(value.tryCast(handle));

    DAVA::NMaterial* material = reflectedCast<DAVA::NMaterial>(handle.data(), handle.type(), defManager);
    DVASSERT(material != nullptr);
    delegateObj.Exec(Command2::Create<ShowMaterialAction>(material));
}

void EntityInjectDataExtension::AddCustomProperty(const RefPropertyItem* item)
{
    AddCustomPropertyWidget* w = new AddCustomPropertyWidget(DAVA::VariantType::TYPE_STRING, QtMainWindow::Instance());
    w->ValueReady.Connect([this, item](const DAVA::String& name, const DAVA::VariantType& value)
                          {
                              const std::vector<std::shared_ptr<const PropertyNode>>& objects = item->getObjects();
                              delegateObj.StartBatch("Add custom property", static_cast<DAVA::uint32>(objects.size()));

                              for (const std::shared_ptr<const PropertyNode>& object : objects)
                              {
                                  Collection collectionHandle;
                                  if (object->propertyInstance->get(object->object, defManager).tryCast(collectionHandle))
                                  {
                                      CollectionImplPtr impl = collectionHandle.getImpl();
                                      NGTLayer::NGTKeyedArchiveImpl* archImpl = dynamic_cast<NGTLayer::NGTKeyedArchiveImpl*>(impl.get());
                                      DVASSERT(archImpl != nullptr);
                                      DAVA::KeyedArchive* archive = archImpl->GetArchive();
                                      delegateObj.Exec(Command2::Create<KeyedArchiveAddValueCommand>(archive, name, value));
                                  }
                              }

                              delegateObj.EndBatch();
                          });
    w->show();
    w->move(300, 300);
}

AddCustomPropertyWidget::AddCustomPropertyWidget(int defaultType, QWidget* parent /* = NULL */)
    : QWidget(parent)
    , presetWidget(nullptr)
{
    QGridLayout* grLayout = new QGridLayout();
    int delautTypeIndex = 0;

    defaultBtn = new QPushButton("Ok", this);
    keyWidget = new QLineEdit(this);
    valueWidget = new QComboBox(this);

    int j = 0;
    for (int type = (DAVA::VariantType::TYPE_NONE + 1); type < DAVA::VariantType::TYPES_COUNT; type++)
    {
        // don't allow byte array
        if (type != DAVA::VariantType::TYPE_BYTE_ARRAY)
        {
            valueWidget->addItem(DAVA::VariantType::variantNamesMap[type].variantName.c_str(), type);

            if (type == defaultType)
            {
                delautTypeIndex = j;
            }

            j++;
        }
    }
    valueWidget->setCurrentIndex(delautTypeIndex);

    int row = 0;
    grLayout->addWidget(new QLabel("Key:", this), row, 0, 1, 1);
    grLayout->addWidget(keyWidget, row, 1, 1, 2);
    grLayout->addWidget(new QLabel("Value type:", this), ++row, 0, 1, 1);
    grLayout->addWidget(valueWidget, row, 1, 1, 2);

    const DAVA::Vector<DAVA::String>& presetValues = EditorConfig::Instance()->GetProjectPropertyNames();
    if (presetValues.size() > 0)
    {
        presetWidget = new QComboBox(this);

        presetWidget->addItem("None", DAVA::VariantType::TYPE_NONE);
        for (size_t i = 0; i < presetValues.size(); ++i)
        {
            presetWidget->addItem(presetValues[i].c_str(), EditorConfig::Instance()->GetPropertyValueType(presetValues[i]));
        }

        grLayout->addWidget(new QLabel("Preset:", this), ++row, 0, 1, 1);
        grLayout->addWidget(presetWidget, row, 1, 1, 2);

        QObject::connect(presetWidget, SIGNAL(activated(int)), this, SLOT(PreSetSelected(int)));
    }
    presetWidget->setMaxVisibleItems(presetWidget->count());

    grLayout->addWidget(defaultBtn, ++row, 2, 1, 1);

    grLayout->setMargin(5);
    grLayout->setSpacing(3);
    setLayout(grLayout);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setWindowOpacity(0.95);

    QObject::connect(defaultBtn, SIGNAL(pressed()), this, SLOT(OkKeyPressed()));
}

void AddCustomPropertyWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    keyWidget->setFocus();
}

void AddCustomPropertyWidget::keyPressEvent(QKeyEvent* e)
{
    if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter))
    {
        switch (e->key())
        {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            defaultBtn->click();
            break;
        case Qt::Key_Escape:
            this->deleteLater();
            break;
        default:
            e->ignore();
            return;
        }
    }
    else
    {
        e->ignore();
    }
}

void AddCustomPropertyWidget::OkKeyPressed()
{
    DAVA::String key = keyWidget->text().toStdString();

    if (key.empty())
    {
        // TODO:
        // other way to report error without losing focus
        // ...
        //

        QMessageBox::warning(nullptr, "Wrong key value", "Key value can't be empty");
    }
    else
    {
        // preset?
        int presetType = DAVA::VariantType::TYPE_NONE;
        if (presetWidget != nullptr)
        {
            presetType = presetWidget->itemData(presetWidget->currentIndex()).toInt();
        }

        if (DAVA::VariantType::TYPE_NONE != presetType)
        {
            DAVA::VariantType presetValue = *(EditorConfig::Instance()->GetPropertyDefaultValue(key));
            ValueReady.Emit(key, presetValue);
        }
        else
        {
            ValueReady.Emit(key, DAVA::VariantType::FromType(valueWidget->itemData(valueWidget->currentIndex()).toInt()));
        }

        this->deleteLater();
    }
}

void AddCustomPropertyWidget::PreSetSelected(int index)
{
    if (presetWidget->itemData(index).toInt() != DAVA::VariantType::TYPE_NONE)
    {
        keyWidget->setText(presetWidget->itemText(index));
        keyWidget->setEnabled(false);
        valueWidget->setEnabled(false);
    }
    else
    {
        keyWidget->setText("");
        keyWidget->setEnabled(true);
        valueWidget->setEnabled(true);
    }
}
