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
    void ExposeChildren(const std::shared_ptr<PropertyNode>&, Vector<std::shared_ptr<PropertyNode>>&) const override
    {
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

    void ProduceCommand(const std::shared_ptr<PropertyNode>& node, const Any& newValue)
    {
    }

    void ProduceCommand(const Reflection::Field& object, const Any& newValue)
    {
    }

    void Exec(std::unique_ptr<Command>&&)
    {
    }

    void EndBatch()
    {
    }
};
}

std::shared_ptr<PropertyNode> MakeRootNode(IChildAllocator* allocator, DAVA::Reflection::Field&& field)
{
    return allocator->CreatePropertyNode(nullptr, std::move(field), 0, PropertyNode::SelfRoot);
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

DAVA::String PropertyNode::BuildID() const
{
    String key = field.key.Cast<String>();
    if (idPostfix.IsValid())
    {
        return key + idPostfix.c_str();
    }

    return key;
}

const int32 PropertyNode::InvalidSortKey = std::numeric_limits<int32>::max();
const int32 PropertyNode::FavoritesRootSortKey = -1000;

ChildCreatorExtension::ChildCreatorExtension()
    : ExtensionChain(Type::Instance<ChildCreatorExtension>())
{
}

void ChildCreatorExtension::ExposeChildren(const std::shared_ptr<PropertyNode>& node, Vector<std::shared_ptr<PropertyNode>>& children) const
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

bool ChildCreatorExtension::CanBeExposed(const Reflection::Field& field) const
{
    if (field.ref.GetMeta<M::HiddenField>() != nullptr)
    {
        return false;
    }

    if (field.ref.GetMeta<M::DeveloperModeOnly>() != nullptr && IsDeveloperMode() == false)
    {
        return false;
    }

    return true;
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

ModifyExtension::MultiCommandInterface ModifyExtension::GetMultiCommandInterface(const String& description, uint32 commandCount)
{
    BeginBatch(description, commandCount);
    return MultiCommandInterface(std::shared_ptr<ModifyExtension>(this, ModifyExtDeleter()));
}

void ModifyExtension::BeginBatch(const String& text, uint32 commandCount)
{
    GetNext<ModifyExtension>()->BeginBatch(text, commandCount);
}

void ModifyExtension::ProduceCommand(const std::shared_ptr<PropertyNode>& node, const Any& newValue)
{
    GetNext<ModifyExtension>()->ProduceCommand(node, newValue);
}

void ModifyExtension::ProduceCommand(const Reflection::Field& object, const Any& newValue)
{
    GetNext<ModifyExtension>()->ProduceCommand(object, newValue);
}

void ModifyExtension::Exec(std::unique_ptr<Command>&& command)
{
    GetNext<ModifyExtension>()->Exec(std::move(command));
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
    extension->ProduceCommand(node, newValue);
    if (node->field.ref.IsValid())
    {
        node->cachedValue = node->field.ref.GetValue();
    }
    else
    {
        node->cachedValue = Any();
    }
}

void ModifyExtension::MultiCommandInterface::Exec(std::unique_ptr<DAVA::Command>&& command)
{
    extension->Exec(std::move(command));
}

void ModifyExtension::MultiCommandInterface::ProduceCommand(const Reflection::Field& object, const Any& newValue)
{
    extension->ProduceCommand(object, newValue);
}

} // namespace TArc
} // namespace DAVA