#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/EmptyComponentValue.h"

namespace DAVA
{
namespace TArc
{
namespace PMEDetails
{
class DummyChildCreator : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<const PropertyNode>&, Vector<std::shared_ptr<PropertyNode>>&) const override
    {
    }
};

class DummyMerger : public MergeValuesExtension
{
public:
    ReflectedPropertyItem* LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const override
    {
        return nullptr;
    }
};

class DummyEditorComponent : public EditorComponentExtension
{
public:
    std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const override
    {
        return std::make_unique<EmptyComponentValue>();
    }
};

class DummyModifyExtension : public ModifyExtension
{
public:
    void ProduceCommand(const Vector<Reflection::Field>&, const Any&) override
    {
    }
};
}

std::shared_ptr<PropertyNode> MakeRootNode(IChildAllocator* allocator, DAVA::Reflection::Field&& field)
{
    return allocator->CreatePropertyNode(std::move(field), PropertyNode::SelfRoot);
}

bool PropertyNode::operator==(const PropertyNode& other) const
{
    return propertyType == other.propertyType &&
    field.ref.GetValueObject() == other.field.ref.GetValueObject() &&
    cachedValue == other.cachedValue &&
    field.key == other.field.key;
}

bool PropertyNode::operator!=(const PropertyNode& other) const
{
    return propertyType != other.propertyType ||
    field.ref.GetValueObject() != other.field.ref.GetValueObject() ||
    cachedValue != other.cachedValue ||
    field.key != field.key;
}

ChildCreatorExtension::ChildCreatorExtension()
    : ExtensionChain(Type::Instance<ChildCreatorExtension>())
{
}

void ChildCreatorExtension::ExposeChildren(const std::shared_ptr<const PropertyNode>& node, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    GetNext<ChildCreatorExtension>()->ExposeChildren(node, children);
}

std::shared_ptr<ChildCreatorExtension> ChildCreatorExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyChildCreator>();
}

void ChildCreatorExtension::SetAllocator(std::shared_ptr<IChildAllocator> allocator_)
{
    allocator = allocator_;
}

MergeValuesExtension::MergeValuesExtension()
    : ExtensionChain(Type::Instance<MergeValuesExtension>())
{
}

ReflectedPropertyItem* MergeValuesExtension::LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const
{
    return GetNext<MergeValuesExtension>()->LookUpItem(node, items);
}

std::shared_ptr<MergeValuesExtension> MergeValuesExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyMerger>();
}

EditorComponentExtension::EditorComponentExtension()
    : ExtensionChain(Type::Instance<EditorComponentExtension>())
{
}

std::unique_ptr<BaseComponentValue> EditorComponentExtension::GetEditor(const std::shared_ptr<const PropertyNode>& node) const
{
    return GetNext<EditorComponentExtension>()->GetEditor(node);
}

std::shared_ptr<EditorComponentExtension> EditorComponentExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyEditorComponent>();
}

ModifyExtension::ModifyExtension()
    : ExtensionChain(Type::Instance<ModifyExtension>())
{
}

void ModifyExtension::ModifyPropertyValue(Vector<std::shared_ptr<PropertyNode>>& nodes, const Any& newValue)
{
    Vector<Reflection::Field> reflections;
    for (std::shared_ptr<PropertyNode>& node : nodes)
    {
        reflections.push_back(node->field);
    }

    ProduceCommand(reflections, newValue);

    for (std::shared_ptr<PropertyNode>& node : nodes)
    {
        if (node->field.ref.IsValid())
        {
            node->cachedValue = node->field.ref.GetValue();
        }
        else
        {
            node->cachedValue = Any();
        }
    }
}

void ModifyExtension::ProduceCommand(const Vector<Reflection::Field>& objects, const Any& newValue)
{
    GetNext<ModifyExtension>()->ProduceCommand(objects, newValue);
}

std::shared_ptr<ModifyExtension> ModifyExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyModifyExtension>();
}

} // namespace TArc
} // namespace DAVA