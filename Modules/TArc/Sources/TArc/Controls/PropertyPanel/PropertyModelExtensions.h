#pragma once

#include "Reflection/Reflection.h"
#include "Base/BaseTypes.h"
#include "Base/Type.h"
#include "Base/Any.h"

#include <memory>

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyItem;
class BaseComponentValue;

struct PropertyNode
{
public:
    enum PropertyType : int32
    {
        Invalid = -1,
        SelfRoot = 0,
        RealProperty,
        GroupProperty,
        VirtualProperty, // reserve some range for generic types. I don't know now what types it will be,
        // but reserve some values is good idea in my opinion
        DomainSpecificProperty = 255
        // use can use values DomainSpecificProperty, DomainSpecificProperty + 1, ... , DomainSpecificProperty + n
        // on you own purpose. It's only way to transfer some information between iterations
    };

    PropertyNode() = default;

    bool operator==(const PropertyNode& other) const;
    bool operator!=(const PropertyNode& other) const;

    int32 propertyType = Invalid; // it can be value from PropertyType or any value that you set in your extension
    Reflection::Field field;
    Any cachedValue;
};

class IChildAllocator
{
public:
    virtual ~IChildAllocator() = default;
    virtual std::shared_ptr<PropertyNode> CreatePropertyNode(Reflection::Field&& reflection, int32_t type = PropertyNode::RealProperty) = 0;
};

std::shared_ptr<PropertyNode> MakeRootNode(IChildAllocator* allocator, DAVA::Reflection::Field&& field);

class ExtensionChain
{
public:
    ExtensionChain(const Type* type)
        : extensionType(type)
    {
    }

    virtual ~ExtensionChain()
    {
        nextExtension.reset();
    }

    static std::shared_ptr<ExtensionChain> AddExtension(std::shared_ptr<ExtensionChain> head, const std::shared_ptr<ExtensionChain>& extension)
    {
        if (extension->extensionType != head->extensionType)
        {
            DVASSERT(false);
            return head;
        }

        extension->nextExtension = head;
        DVASSERT(extension->nextExtension != nullptr);
        return extension;
    }

    static std::shared_ptr<ExtensionChain> RemoveExtension(std::shared_ptr<ExtensionChain> head, const std::shared_ptr<ExtensionChain>& extension)
    {
        if (extension->extensionType != head->extensionType)
        {
            DVASSERT(false);
            return head;
        }

        if (head == extension)
        {
            std::shared_ptr<ExtensionChain> result = extension->nextExtension;
            extension->nextExtension.reset();
            return result;
        }

        head->nextExtension = RemoveExtension(head->nextExtension, extension);
        return head;
    }

    const Type* GetType() const
    {
        return extensionType;
    }

protected:
    template <typename T>
    T* GetNext()
    {
        DVASSERT(nextExtension != nullptr);
        DVASSERT(dynamic_cast<T*>(nextExtension.get()));
        return static_cast<T*>(nextExtension.get());
    }

    template <typename T>
    const T* GetNext() const
    {
        DVASSERT(nextExtension != nullptr);
        DVASSERT(dynamic_cast<const T*>(nextExtension.get()));
        return static_cast<const T*>(nextExtension.get());
    }

private:
    const Type* extensionType;
    std::shared_ptr<ExtensionChain> nextExtension;
};

// The main goal of this extension is create children of some property.
// parent - is property node, that you should create children for.
// children - return value
// use allocator to create children
class ChildCreatorExtension : public ExtensionChain
{
public:
    ChildCreatorExtension();
    virtual void ExposeChildren(const std::shared_ptr<const PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const;
    static std::shared_ptr<ChildCreatorExtension> CreateDummy();

    void SetAllocator(std::shared_ptr<IChildAllocator> allocator);

protected:
    std::shared_ptr<IChildAllocator> allocator;
};

// This extension should implements custom rules of search ReflectedPropertyItem for some value.
// As ReflectedPropertyItem is real item that user will see, in multi selection case we should make decision in which
// ReflectedPropertyItem add current property.
// Limitation - node.propertyInstance should be equal of item.property()
// If your extension return nullptr, ReflectedPropertyModel will create new ReflectedPropertyItem for that.
class MergeValuesExtension : public ExtensionChain
{
public:
    MergeValuesExtension();
    virtual ReflectedPropertyItem* LookUpItem(const std::shared_ptr<const PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& items) const;
    static std::shared_ptr<MergeValuesExtension> CreateDummy();
};

class EditorComponentExtension : public ExtensionChain
{
public:
    EditorComponentExtension();
    virtual std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const;
    static std::shared_ptr<EditorComponentExtension> CreateDummy();
};

class ModifyExtension : public ExtensionChain
{
public:
    ModifyExtension();
    void ModifyPropertyValue(Vector<std::shared_ptr<PropertyNode>>& nodes, const Any& newValue);
    virtual void ProduceCommand(const Vector<Reflection::Field>& objects, const Any& newValue);
    static std::shared_ptr<ModifyExtension> CreateDummy();
};
} // namespace TArc
} // namespace DAVA
