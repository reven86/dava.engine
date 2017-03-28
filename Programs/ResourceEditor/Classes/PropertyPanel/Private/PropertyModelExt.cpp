#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Commands2/SetFieldValueCommand.h"
#include "Classes/Commands2/AddComponentCommand.h"
#include "Classes/PropertyPanel/PropertyPanelCommon.h"

#include <TArc/Controls/PropertyPanel/PropertyPanelMeta.h>
#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>
#include <TArc/Controls/ComboBox.h>
#include "TArc/Controls/Widget.h"
#include <TArc/Controls/CommonStrings.h>
#include <TArc/Utils/ReflectionHelpers.h>
#include <TArc/Utils/QtConnections.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <Engine/PlatformApi.h>
#include <Scene3D/Entity.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedTypeDB.h>
#include "Reflection/ReflectedMeta.h"
#include <Functional/Function.h>
#include <Base/Any.h>
#include <Base/StaticSingleton.h>

#include <QToolButton>
#include <QHBoxLayout>
#include <QPalette>

namespace PropertyModelExtDetails
{
using namespace DAVA;
using namespace DAVA::TArc;

struct ComponentCreator : public StaticSingleton<ComponentCreator>
{
    const ReflectedType* componentType = nullptr;
};

const char* chooseComponentTypeString = "Choose component type";

struct TypeInitializer : public StaticSingleton<ComponentCreator>
{
    using TypePair = std::pair<String, const ReflectedType*>;
    struct TypePairLess
    {
        bool operator()(const TypePair& p1, const TypePair& p2) const
        {
            if (p1.second == nullptr && p2.second == nullptr)
            {
                return false;
            }

            if (p2.second == nullptr)
            {
                return false;
            }

            if (p1.second == nullptr)
            {
                return true;
            }

            return p1.first < p2.first;
        }
    };
    TypeInitializer()
    {
        AnyCast<TypePair, String>::Register([](const Any& v) -> String
                                            {
                                                return v.Get<TypePair>().first;
                                            });

        types.emplace(String(chooseComponentTypeString), nullptr);

        InitDerivedTypes(Type::Instance<DAVA::Component>());
    }

    void InitDerivedTypes(const Type* type)
    {
        const TypeInheritance* inheritance = type->GetInheritance();
        Vector<TypeInheritance::Info> derivedTypes = inheritance->GetDerivedTypes();
        for (const TypeInheritance::Info& derived : derivedTypes)
        {
            const ReflectedType* refType = ReflectedTypeDB::GetByType(derived.type);
            if (refType == nullptr)
            {
                continue;
            }

            const std::unique_ptr<ReflectedMeta>& meta = refType->GetStructure()->meta;
            if (meta != nullptr && meta->HasMeta<M::CantBeCreatedManualyComponent>())
            {
                continue;
            }

            if (refType->GetCtor(derived.type->Pointer()) != nullptr)
            {
                types.emplace(refType->GetPermanentName(), refType);
            }

            InitDerivedTypes(derived.type);
        }
    }

    Set<TypePair, TypePairLess> types;
};

class ComponentCreatorComponentValue : public BaseComponentValue
{
public:
    ComponentCreatorComponentValue() = default;

protected:
    Any GetMultipleValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override
    {
        return false;
    }

    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const override
    {
        Widget* w = new Widget(parent);
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setMargin(0);
        layout->setSpacing(2);
        w->SetLayout(layout);

        ComponentCreatorComponentValue* nonConstThis = const_cast<ComponentCreatorComponentValue*>(this);
        nonConstThis->toolButton = new QToolButton(w->ToWidgetCast());
        nonConstThis->toolButton->setIcon(SharedIcon(":/QtIcons/addcomponent.png"));
        nonConstThis->toolButton->setIconSize(QSize(12, 12));
        nonConstThis->toolButton->setToolTip(QStringLiteral("Add component"));
        nonConstThis->toolButton->setAutoRaise(true);
        layout->addWidget(nonConstThis->toolButton.data());
        nonConstThis->connections.AddConnection(nonConstThis->toolButton.data(), &QToolButton::clicked, MakeFunction(nonConstThis, &ComponentCreatorComponentValue::AddComponent));

        ControlDescriptorBuilder<ComboBox::Fields> descr;
        descr[ComboBox::Fields::Enumerator] = "types";
        descr[ComboBox::Fields::Value] = "currentType";
        w->AddControl(new ComboBox(descr, wrappersProcessor, model, w->ToWidgetCast()));

        return w;
    }

private:
    TypeInitializer::TypePair GetType() const
    {
        const ReflectedType* type = ComponentCreator::Instance()->componentType;
        if (type == nullptr)
        {
            toolButton->setEnabled(false);
            return TypeInitializer::TypePair(String(chooseComponentTypeString), nullptr);
        }

        toolButton->setEnabled(true);
        return TypeInitializer::TypePair(type->GetPermanentName(), type);
    }

    void SetType(const TypeInitializer::TypePair& type)
    {
        toolButton->setEnabled(type.second != nullptr);
        ComponentCreator::Instance()->componentType = type.second;
    }

    const Set<TypeInitializer::TypePair, TypeInitializer::TypePairLess>& GetTypes() const
    {
        static TypeInitializer t;
        return t.types;
    }

    void AddComponent()
    {
        ComponentCreator* componentCreator = ComponentCreator::Instance();
        const ReflectedType* componentType = componentCreator->componentType;
        DVASSERT(componentType != nullptr);

        String description = Format("Add component: %s", componentType->GetPermanentName().c_str());
        ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(description, static_cast<uint32>(nodes.size()));
        for (std::shared_ptr<PropertyNode>& node : nodes)
        {
            Entity* entity = node->field.ref.GetValueObject().GetPtr<Entity>();
            Any newComponent = componentType->CreateObject(ReflectedType::CreatePolicy::ByPointer);
            Component* component = newComponent.Cast<Component*>();

            cmdInterface.Exec(std::make_unique<AddComponentCommand>(entity, component));
        }

        componentCreator->componentType = nullptr;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ComponentCreatorComponentValue, DAVA::TArc::BaseComponentValue)
    {
        DAVA::ReflectionRegistrator<ComponentCreatorComponentValue>::Begin()
        .Field("currentType", &ComponentCreatorComponentValue::GetType, &ComponentCreatorComponentValue::SetType)
        .Field("types", &ComponentCreatorComponentValue::GetTypes, nullptr)
        .End();
    }

    QPointer<QToolButton> toolButton;
    QtConnections connections;
};
}

namespace DAVA
{
template <>
struct AnyCompare<PropertyModelExtDetails::TypeInitializer::TypePair>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        using T = PropertyModelExtDetails::TypeInitializer::TypePair;
        return v1.Get<T>().second == v2.Get<T>().second;
    }
};
} // namespace DAVA

REModifyPropertyExtension::REModifyPropertyExtension(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
}

void REModifyPropertyExtension::ProduceCommand(const DAVA::Reflection::Field& field, const DAVA::Any& newValue)
{
    GetScene()->Exec(std::make_unique<SetFieldValueCommand>(field, newValue));
}

void REModifyPropertyExtension::Exec(std::unique_ptr<DAVA::Command>&& command)
{
    GetScene()->Exec(std::move(command));
}

void REModifyPropertyExtension::EndBatch()
{
    GetScene()->EndBatch();
}

void REModifyPropertyExtension::BeginBatch(const DAVA::String& text, DAVA::uint32 commandCount)
{
    GetScene()->BeginBatch(text, commandCount);
}

SceneEditor2* REModifyPropertyExtension::GetScene() const
{
    using namespace DAVA::TArc;
    DataContext* ctx = accessor->GetActiveContext();
    DVASSERT(ctx != nullptr);

    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);

    return data->GetScene().Get();
}

void EntityChildCreator::ExposeChildren(const std::shared_ptr<const DAVA::TArc::PropertyNode>& parent, DAVA::Vector<std::shared_ptr<DAVA::TArc::PropertyNode>>& children) const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    if (parent->propertyType == PropertyPanel::AddComponentProperty)
    {
        return;
    }

    if (parent->propertyType == PropertyNode::SelfRoot &&
        parent->cachedValue.GetType() == DAVA::Type::Instance<DAVA::Entity*>())
    {
        DAVA::Reflection::Field f(Any("Entity"), Reflection(parent->field.ref), nullptr);
        std::shared_ptr<PropertyNode> entityNode = allocator->CreatePropertyNode(std::move(f), PropertyNode::GroupProperty);
        entityNode->sortKey = -1; // zero sort key is for favorites root
        children.push_back(entityNode);

        {
            DAVA::Reflection componentsField = f.ref.GetField(DAVA::Entity::componentFieldString);
            DVASSERT(componentsField.IsValid());

            DAVA::TArc::ForEachField(componentsField, [this, &children](Reflection::Field&& field)
                                     {
                                         if (!field.ref.HasMeta<M::HiddenField>())
                                         {
                                             Any value = field.ref.GetValue();
                                             DAVA::Reflection::Field f(GetValueReflectedType(field.ref)->GetPermanentName(), Reflection(field.ref), nullptr);
                                             std::shared_ptr<PropertyNode> node = allocator->CreatePropertyNode(std::move(f), PropertyNode::RealProperty);
                                             DVASSERT(value.CanGet<Component*>());
                                             Component* component = value.Get<Component*>();
                                             node->sortKey = static_cast<size_t>(component->GetType());
                                             children.push_back(node);
                                         }
                                     });

            Reflection::Field addComponentField;
            addComponentField.key = "Add Component";
            addComponentField.ref = parent->field.ref;
            std::shared_ptr<PropertyNode> addComponentNode = allocator->CreatePropertyNode(std::move(addComponentField), PropertyPanel::AddComponentProperty);
            addComponentNode->sortKey = DAVA::TArc::PropertyNode::InvalidSortKey - 1;
            children.push_back(addComponentNode);
        }
    }
    else if (parent->propertyType == PropertyNode::GroupProperty &&
             parent->cachedValue.GetType() == DAVA::Type::Instance<DAVA::Entity*>())
    {
        DAVA::TArc::ForEachField(parent->field.ref, [this, &children](Reflection::Field&& field)
                                 {
                                     if (field.ref.GetValueType() != DAVA::Type::Instance<DAVA::Vector<DAVA::Component*>>() && field.ref.HasMeta<M::HiddenField>() == false)
                                     {
                                         children.push_back(allocator->CreatePropertyNode(std::move(field), PropertyNode::RealProperty));
                                     }
                                 });
    }
    else
    {
        ChildCreatorExtension::ExposeChildren(parent, children);
    }
}

std::unique_ptr<DAVA::TArc::BaseComponentValue> EntityEditorCreator::GetEditor(const std::shared_ptr<const DAVA::TArc::PropertyNode>& node) const
{
    if (node->propertyType == PropertyPanel::AddComponentProperty)
    {
        std::unique_ptr<DAVA::TArc::BaseComponentValue> editor = std::make_unique<PropertyModelExtDetails::ComponentCreatorComponentValue>();
        DAVA::TArc::BaseComponentValue::Style style;
        style.fontBold = true;
        style.fontItalic = true;
        style.fontColor = QPalette::ButtonText;
        style.bgColor = QPalette::AlternateBase;
        editor->SetStyle(style);
        return std::move(editor);
    }

    const DAVA::Type* valueType = node->cachedValue.GetType();
    static const DAVA::Type* componentType = DAVA::Type::Instance<DAVA::Component*>();
    static const DAVA::Type* entityType = DAVA::Type::Instance<DAVA::Entity*>();

    if ((node->propertyType == DAVA::TArc::PropertyNode::GroupProperty && valueType == entityType)
        || node->cachedValue.GetType() == DAVA::Type::Instance<DAVA::Component*>())
    {
        std::unique_ptr<DAVA::TArc::BaseComponentValue> editor = EditorComponentExtension::GetEditor(node);
        DAVA::TArc::BaseComponentValue::Style style;
        style.fontBold = true;
        style.fontColor = QPalette::ButtonText;
        style.bgColor = QPalette::AlternateBase;
        editor->SetStyle(style);
        return std::move(editor);
    }

    return EditorComponentExtension::GetEditor(node);
}
