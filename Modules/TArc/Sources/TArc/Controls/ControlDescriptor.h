#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>

#include <Debug/DVAssert.h>

namespace DAVA
{
namespace TArc
{
class DescriptorNode
{
public:
    DescriptorNode();

    DescriptorNode& operator=(const char* name);
    DescriptorNode& operator=(const String& name);
    DescriptorNode& operator=(const FastName& name);

    const FastName& GetName() const;

private:
    FastName fieldName;
};

inline const FastName& DescriptorNode::GetName() const
{
    return fieldName;
}

template <typename Enum>
class ControlDescriptorBuilder
{
public:
    ControlDescriptorBuilder();
    DescriptorNode& operator[](Enum enumValue);

    uint32 GetMaxValue() const;

private:
    friend class ControlDescriptor;
    Map<Enum, DescriptorNode> nodes;
};

template <typename Enum>
ControlDescriptorBuilder<Enum>::ControlDescriptorBuilder()
{
    static_assert(std::is_enum<Enum>::value, "Template parameter Enum should be enum or enum class");
}

template <typename Enum>
DescriptorNode& ControlDescriptorBuilder<Enum>::operator[](Enum value)
{
    return nodes[value];
}

template <typename Enum>
uint32 ControlDescriptorBuilder<Enum>::GetMaxValue() const
{
    return static_cast<uint32>(Enum::FieldCount);
}

class ControlDescriptor
{
public:
    struct Field
    {
        FastName name;
        bool isChanged = false;
    };

    template <typename Enum>
    ControlDescriptor(const ControlDescriptorBuilder<Enum>& descriptor);

    template <typename Enum>
    bool GetValue(const FastName& name, Enum& value) const;
    template <typename Enum>
    const FastName& GetName(Enum value) const;
    template <typename Enum>
    bool IsChanged(Enum value) const;

    Vector<Field> fieldNames;
};

template <typename Enum>
ControlDescriptor::ControlDescriptor(const ControlDescriptorBuilder<Enum>& descriptor)
{
    fieldNames.resize(descriptor.GetMaxValue());
    DVASSERT(descriptor.nodes.size() <= static_cast<size_t>(descriptor.GetMaxValue()));

    for (const auto& iter : descriptor.nodes)
    {
        Field& f = fieldNames[static_cast<size_t>(iter.first)];
        f.name = iter.second.GetName();
    }
}

template <typename Enum>
bool ControlDescriptor::GetValue(const FastName& name, Enum& value) const
{
    auto iter = std::find_if(fieldNames.begin(), fieldNames.end(), [name](const Field& f)
                             {
                                 return f.name == name;
                             });

    if (iter == fieldNames.end())
    {
        return false;
    }

    value = static_cast<Enum>(std::distance(fieldNames.begin(), iter));
    return true;
}

template <typename Enum>
const FastName& ControlDescriptor::GetName(Enum value) const
{
    size_t index = static_cast<size_t>(value);
    DVASSERT(index < fieldNames.size());

    return fieldNames[index].name;
}

template <typename Enum>
bool ControlDescriptor::IsChanged(Enum value) const
{
    size_t index = static_cast<size_t>(value);
    DVASSERT(index < fieldNames.size());

    return fieldNames[index].isChanged;
}

} // namespace TArc
} // namespace DAVA
