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
    void BeginBatch(const String& text, uint32 commandCount)
    {
    }

    void ProduceCommand(const Reflection::Field& object, const Any& newValue)
    {
    }

    void EndBatch()
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

struct ModifyExtension::ModifyExtDeleter
{
    ModifyExtDeleter() = default;

    void operator()(ModifyExtension* ext) const
    {
        ext->EndBatch();
    }
};

ModifyExtension::ModifyExtension()
    : ExtensionChain(Type::Instance<ModifyExtension>())
{
}

void ModifyExtension::ModifyPropertyValue(const Vector<std::shared_ptr<PropertyNode>>& nodes, const Any& newValue)
{
    MultiCommandInterface modif = GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    for (std::shared_ptr<PropertyNode> node : nodes)
    {
        modif.ModifyPropertyValue(node, newValue);
    }
}

ModifyExtension::MultiCommandInterface ModifyExtension::GetMultiCommandInterface(uint32 commandCount)
{
    BeginBatch("Set property value", commandCount);
    return MultiCommandInterface(std::shared_ptr<ModifyExtension>(this, ModifyExtDeleter()));
}

void ModifyExtension::BeginBatch(const String& text, uint32 commandCount)
{
    GetNext<ModifyExtension>()->BeginBatch(text, commandCount);
}

void ModifyExtension::ProduceCommand(const Reflection::Field& object, const Any& newValue)
{
    GetNext<ModifyExtension>()->ProduceCommand(object, newValue);
}

void ModifyExtension::EndBatch()
{
    GetNext<ModifyExtension>()->EndBatch();
}

std::shared_ptr<ModifyExtension> ModifyExtension::CreateDummy()
{
    return std::make_shared<PMEDetails::DummyModifyExtension>();
}

ModifyExtension::MultiCommandInterface::MultiCommandInterface(std::shared_ptr<ModifyExtension> ext)
    : extension(ext)
{
}

void ModifyExtension::MultiCommandInterface::ModifyPropertyValue(const std::shared_ptr<PropertyNode>& node, const Any& newValue)
{
    extension->ProduceCommand(node->field, newValue);
    if (node->field.ref.IsValid())
    {
        node->cachedValue = node->field.ref.GetValue();
    }
    else
    {
        node->cachedValue = Any();
    }
}

} // namespace TArc
} // namespace DAVA