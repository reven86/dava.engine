#if 0
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

class ProxyProperty : public wgt::BaseProperty
{
public:
    ProxyProperty(const char* name)
        : BaseProperty(name, wgt::TypeId::getType<wgt::ObjectHandle>())
    {
    }

    wgt::Variant get(const wgt::ObjectHandle& handle, const wgt::IDefinitionManager&) const override
    {
        return handle;
    }
};

namespace ExtensionsDetails
{
const wgt::TypeId entityType = wgt::TypeId::getType<DAVA::Entity>();
const wgt::TypeId objectHandleType = wgt::TypeId::getType<wgt::ObjectHandle>();
const wgt::TypeId nmaterialType = wgt::TypeId::getType<DAVA::NMaterial*>();
const wgt::TypeId keyedArchive = wgt::TypeId::getType<DAVA::KeyedArchive*>();
const wgt::TypeId renderBatchType = wgt::TypeId::getType<DAVA::RenderBatch>();

DAVA::RenderBatch* ExtractRenderBatch(const wgt::RefPropertyItem* item, wgt::IDefinitionManager& defManager)
{
    const std::vector<std::shared_ptr<const wgt::PropertyNode>>& objects = item->getObjects();
    DVASSERT(objects.size() == 1);

    std::shared_ptr<const wgt::PropertyNode> node = objects.front();
    wgt::ObjectHandle valueHandle;
    DVVERIFY(node->propertyInstance->get(node->object, defManager).tryCast(valueHandle));
    DAVA::RenderBatch* batch = valueHandle.getBase<DAVA::RenderBatch>();
    DVASSERT(batch != nullptr);

    return batch;
}

DAVA::Entity* FindEntityWithRenderObject(const wgt::RefPropertyItem* item, DAVA::RenderObject* renderObject)
{
    const wgt::RefPropertyItem* parent = item->getParent();
    while (parent != nullptr)
    {
        item = parent;
        parent = item->getParent();
    }

    const std::vector<std::shared_ptr<const wgt::PropertyNode>>& objects = item->getObjects();
    DVASSERT(objects.size() == 1);
    std::shared_ptr<const wgt::PropertyNode> object = objects.front();
    DAVA::Entity* entity = object->object.getBase<DAVA::Entity>();

    DVASSERT(GetRenderObject(entity) == renderObject);

    return entity;
}
}

std::string BuildCollectionElementName(const wgt::Collection::ConstIterator& iter, wgt::IDefinitionManager& defMng)
{
    wgt::Variant v = iter.value();
    wgt::ObjectHandle handle;
    if (v.tryCast(handle))
    {
        const wgt::IClassDefinition* definition = handle.getDefinition(defMng);
        DVASSERT(definition);

        const wgt::MetaDisplayNameObj* displayData = wgt::findFirstMetaData<wgt::MetaDisplayNameObj>(*definition, defMng);
        return DAVA::WStringToString(DAVA::WideString(displayData->getDisplayName()));
    }
    else
    {
        return iter.key().cast<std::string>();
    }
}

void EntityChildCreatorExtension::exposeChildren(const std::shared_ptr<const wgt::PropertyNode>& node, std::vector<std::shared_ptr<const wgt::PropertyNode>>& children, wgt::IDefinitionManager& defMng) const
{
    if (node->propertyType == DAVAProperiesEnum::EntityRoot)
    {
        std::shared_ptr<const wgt::PropertyNode> newNode = allocator->createPropertyNode(node->propertyInstance, node->object);
        ChildCreatorExtension::exposeChildren(newNode, children, defMng);

        for (auto iter = children.rbegin(); iter != children.rend(); ++iter)
        {
            const std::shared_ptr<const wgt::PropertyNode> child = *iter;
            if (strcmp(child->propertyInstance->getName(), "components") == 0)
            {
                auto forwardIter = (iter + 1).base();
                children.erase(forwardIter);
                break;
            }
        }
    }
    else if (node->object.type() == ExtensionsDetails::entityType && node->propertyType == wgt::PropertyNode::SelfRoot)
    {
        static wgt::IBasePropertyPtr entityProxy = std::make_shared<ProxyProperty>("Entity");
        children.push_back(allocator->createPropertyNode(entityProxy, node->object, DAVAProperiesEnum::EntityRoot));
        DAVA::Entity* entity = node->object.getBase<DAVA::Entity>();
        DVASSERT(entity != nullptr);

        const wgt::IClassDefinition* definition = node->object.getDefinition(defMng);
        DVASSERT(definition != nullptr);
        wgt::IBasePropertyPtr components = definition->findProperty("components");
        DVASSERT(components != nullptr);
        wgt::Variant componentsCollection = components->get(node->object, defMng);

        wgt::Collection collection;
        if (componentsCollection.tryCast(collection))
        {
            for (auto iter = collection.begin(); iter != collection.end(); ++iter)
            {
                wgt::ReflectedIteratorValue value;
                value.iterator = iter;
                value.value = iter.value();

                std::string name = BuildCollectionElementName(iter, defMng);
                wgt::IBasePropertyPtr property = allocator->getCollectionItemProperty(std::move(name), iter.value().type()->typeId(), defMng);
                children.push_back(allocator->createPropertyNode(property, wgt::ObjectHandle(value), wgt::PropertyNode::CollectionItem));
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

wgt::RefPropertyItem* EntityMergeValueExtension::lookUpItem(const std::shared_ptr<const wgt::PropertyNode>& node, const std::vector<std::unique_ptr<wgt::RefPropertyItem>>& items,
                                                            wgt::IDefinitionManager& definitionManager) const
{
    return MergeValuesExtension::lookUpItem(node, items, definitionManager);
}

wgt::Variant PropertyPanelGetExtension::getValue(const wgt::RefPropertyItem* item, int column, size_t roleId, wgt::IDefinitionManager& definitionManager) const
{
    if (column == 1 && roleId == wgt::ValueRole::roleId_)
    {
        wgt::Variant result = SetterGetterExtension::getValue(item, column, roleId, definitionManager);
        wgt::ObjectHandle handle;
        if (result.tryCast(handle))
        {
            wgt::TypeId type = handle.type();
            if (type.isPointer())
            {
                type = type.removePointer();
            }

            return type.getName();
        }
    }
    return SetterGetterExtension::getValue(item, column, roleId, definitionManager);
}

EntityInjectDataExtension::EntityInjectDataExtension(Delegate& delegateObj_, wgt::IComponentContext& context)
    : delegateObj(delegateObj_)
    , defManagerHolder(context)
{
}

void EntityInjectDataExtension::inject(wgt::RefPropertyItem* item)
{
    INTERFACE_REQUEST(wgt::IDefinitionManager, defManager, defManagerHolder, void());

    static wgt::TypeId removableComponents[] = { wgt::TypeId::getType<DAVA::RenderComponent>(), wgt::TypeId::getType<DAVA::ActionComponent>() };
    std::shared_ptr<const wgt::PropertyNode> node = item->getObjects().front();
    wgt::Variant value = node->propertyInstance->get(node->object, defManager);
    wgt::ObjectHandle handle;
    if (value.tryCast(handle))
    {
        wgt::TypeId type = handle.type();
        if (type.isPointer())
            type = type.removePointer();

        if (std::find(std::begin(removableComponents), std::end(removableComponents), type) != std::end(removableComponents))
        {
            std::vector<wgt::ButtonItem> buttons;
            buttons.emplace_back(true, "/QtIcons/remove.png", std::bind(&EntityInjectDataExtension::RemoveComponent, this, item));
            wgt::ButtonsModel* buttonsModel = new wgt::ButtonsModel(std::move(buttons));
            item->injectData(wgt::buttonsDefinitionRole::roleId_, wgt::Variant(wgt::ObjectHandle(std::unique_ptr<wgt::IListModel>(buttonsModel))));
        }
        else if (type == ExtensionsDetails::renderBatchType)
        {
            std::vector<wgt::ButtonItem> buttons;
            buttons.emplace_back(true, "/QtIcons/external.png", std::bind(&EntityInjectDataExtension::RebuildTangentSpace, this, item));
            buttons.emplace_back(true, "/QtIcons/shadow.png", std::bind(&EntityInjectDataExtension::ConvertBatchToShadow, this, item));
            buttons.emplace_back(true, "/QtIcons/remove.png", std::bind(&EntityInjectDataExtension::RemoveRenderBatch, this, item));
            wgt::ButtonsModel* buttonsModel = new wgt::ButtonsModel(std::move(buttons));
            item->injectData(wgt::buttonsDefinitionRole::roleId_, wgt::Variant(wgt::ObjectHandle(std::unique_ptr<wgt::IListModel>(buttonsModel))));
        }
        else if (node->propertyInstance->getType() == ExtensionsDetails::nmaterialType)
        {
            std::vector<wgt::ButtonItem> buttons;
            buttons.emplace_back(true, "/QtIcons/3d.png", std::bind(&EntityInjectDataExtension::OpenMaterials, this, item));
            wgt::ButtonsModel* buttonsModel = new wgt::ButtonsModel(std::move(buttons));
            item->injectData(wgt::buttonsDefinitionRole::roleId_, wgt::Variant(wgt::ObjectHandle(std::unique_ptr<wgt::IListModel>(buttonsModel))));
        }
    }
    else if (node->propertyInstance->getType() == ExtensionsDetails::keyedArchive)
    {
        std::vector<wgt::ButtonItem> buttons;
        buttons.emplace_back(true, "/QtIcons/keyplus.png", std::bind(&EntityInjectDataExtension::AddCustomProperty, this, item));
        wgt::ButtonsModel* buttonsModel = new wgt::ButtonsModel(std::move(buttons));
        item->injectData(wgt::buttonsDefinitionRole::roleId_, wgt::Variant(wgt::ObjectHandle(std::unique_ptr<wgt::IListModel>(buttonsModel))));
    }
}

void EntityInjectDataExtension::updateInjection(wgt::RefPropertyItem* item)
{
    INTERFACE_REQUEST(wgt::IDefinitionManager, defManager, defManagerHolder, void());

    wgt::Variant buttons = item->getInjectedData(wgt::buttonsDefinitionRole::roleId_);
    wgt::ObjectHandle modelHandle;
    if (!buttons.tryCast(modelHandle))
        return;

    wgt::ButtonsModel* model = dynamic_cast<wgt::ButtonsModel*>(modelHandle.getBase<wgt::IListModel>());

    if (model == nullptr)
    {
        return;
    }

    const std::vector<std::shared_ptr<const wgt::PropertyNode>>& objects = item->getObjects();
    DVASSERT(!objects.empty());

    bool isSingleSelection = objects.size() == 1;
    std::shared_ptr<const wgt::PropertyNode> node = objects.front();
    wgt::ObjectHandle valueHandle;

    if (node->propertyInstance->get(node->object, defManager).tryCast(valueHandle))
    {
        wgt::TypeId type = valueHandle.type();
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

void EntityInjectDataExtension::RemoveComponent(const wgt::RefPropertyItem* item)
{
    INTERFACE_REQUEST(wgt::IDefinitionManager, defManager, defManagerHolder, void());

    const std::vector<std::shared_ptr<const wgt::PropertyNode>>& objects = item->getObjects();
    delegateObj.BeginBatch("Remove component", static_cast<DAVA::uint32>(objects.size()));
    for (const std::shared_ptr<const wgt::PropertyNode>& object : objects)
    {
        wgt::Variant value = object->propertyInstance->get(object->object, defManager);
        wgt::ObjectHandle handle;
        DVVERIFY(value.tryCast(handle));

        DAVA::Component* component = wgt::reflectedCast<DAVA::Component>(handle.data(), handle.type(), defManager);
        DVASSERT(component != nullptr);

        delegateObj.Exec(std::unique_ptr<DAVA::Command>(new RemoveComponentCommand(component->GetEntity(), component)));
    }
    delegateObj.EndBatch();
}

void EntityInjectDataExtension::RemoveRenderBatch(const wgt::RefPropertyItem* item)
{
    INTERFACE_REQUEST(wgt::IDefinitionManager, defManager, defManagerHolder, void());
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
    delegateObj.Exec(std::unique_ptr<DAVA::Command>(new DeleteRenderBatchCommand(entity, renderObject, batchIndex)));
}

void EntityInjectDataExtension::ConvertBatchToShadow(const wgt::RefPropertyItem* item)
{
    INTERFACE_REQUEST(wgt::IDefinitionManager, defManager, defManagerHolder, void());
    DAVA::RenderBatch* batch = ExtensionsDetails::ExtractRenderBatch(item, defManager);
    DAVA::Entity* entity = ExtensionsDetails::FindEntityWithRenderObject(item, batch->GetRenderObject());

    delegateObj.Exec(std::unique_ptr<DAVA::Command>(new ConvertToShadowCommand(entity, batch)));
}

void EntityInjectDataExtension::RebuildTangentSpace(const wgt::RefPropertyItem* item)
{
    INTERFACE_REQUEST(wgt::IDefinitionManager, defManager, defManagerHolder, void());
    DAVA::RenderBatch* batch = ExtensionsDetails::ExtractRenderBatch(item, defManager);
    delegateObj.Exec(std::unique_ptr<DAVA::Command>(new RebuildTangentSpaceCommand(batch, true)));
}

void EntityInjectDataExtension::OpenMaterials(const wgt::RefPropertyItem* item)
{
    INTERFACE_REQUEST(wgt::IDefinitionManager, defManager, defManagerHolder, void());
    std::shared_ptr<const wgt::PropertyNode> node = item->getObjects().front();
    wgt::Variant value = node->propertyInstance->get(node->object, defManager);
    wgt::ObjectHandle handle;
    DVVERIFY(value.tryCast(handle));

    DAVA::NMaterial* material = wgt::reflectedCast<DAVA::NMaterial>(handle.data(), handle.type(), defManager);
    DVASSERT(material != nullptr);
    delegateObj.Exec(std::unique_ptr<DAVA::Command>(new ShowMaterialAction(material)));
}

void EntityInjectDataExtension::AddCustomProperty(const wgt::RefPropertyItem* item)
{
    INTERFACE_REQUEST(wgt::IDefinitionManager, defManager, defManagerHolder, void());

    AddCustomPropertyWidget* w = new AddCustomPropertyWidget(DAVA::VariantType::TYPE_STRING, QtMainWindow::Instance());
    w->ValueReady.Connect([this, item, &defManager](const DAVA::String& name, const DAVA::VariantType& value)
                          {
                              const std::vector<std::shared_ptr<const wgt::PropertyNode>>& objects = item->getObjects();
                              delegateObj.BeginBatch("Add custom property", static_cast<DAVA::uint32>(objects.size()));

                              for (const std::shared_ptr<const wgt::PropertyNode>& object : objects)
                              {
                                  wgt::Collection collectionHandle;
                                  if (object->propertyInstance->get(object->object, defManager).tryCast(collectionHandle))
                                  {
                                      wgt::CollectionImplPtr impl = collectionHandle.impl();
                                      NGTLayer::NGTKeyedArchiveImpl* archImpl = dynamic_cast<NGTLayer::NGTKeyedArchiveImpl*>(impl.get());
                                      DVASSERT(archImpl != nullptr);
                                      DAVA::KeyedArchive* archive = archImpl->GetArchive();
                                      delegateObj.Exec(std::unique_ptr<DAVA::Command>(new KeyedArchiveAddValueCommand(archive, name, value)));
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

#endif 0
