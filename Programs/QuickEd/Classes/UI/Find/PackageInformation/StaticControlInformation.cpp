#include "UI/Find/PackageInformation/StaticControlInformation.h"
#include "UI/Find/PackageInformation/StaticPackageInformation.h"

using namespace DAVA;

StaticControlInformation::StaticControlInformation(const FastName& name_)
    : name(name_)
{
}

StaticControlInformation::StaticControlInformation(const StaticControlInformation& other)
    : StaticControlInformation(other, other.name, std::shared_ptr<StaticPackageInformation>(), FastName())
{
}

StaticControlInformation::StaticControlInformation(const StaticControlInformation& other, const FastName& name_, const std::shared_ptr<StaticPackageInformation> prototypePackage_, const FastName& prototype_)
    : name(name_)
    , prototypePackage(prototypePackage_)
    , prototype(prototype_)
    , componentCount(other.componentCount)
    , controlProperties(other.controlProperties)
    , componentProperties(other.componentProperties)
{
    for (const std::shared_ptr<StaticControlInformation>& otherChild : other.children)
    {
        std::shared_ptr<StaticControlInformation> child = std::make_shared<StaticControlInformation>(*otherChild);
        child->SetParent(this);
        children.push_back(child);
    }
}

FastName StaticControlInformation::GetName() const
{
    return name;
}

FastName StaticControlInformation::GetPrototype() const
{
    return prototype;
}

String StaticControlInformation::GetPrototypePackagePath() const
{
    return prototypePackage->GetPath();
}

bool StaticControlInformation::HasErrors() const
{
    return results.HasErrors();
}

bool StaticControlInformation::HasComponent(UIComponent::eType componentType) const
{
    const UnorderedMap<UIComponent::eType, int32>::const_iterator iter = componentCount.find(componentType);

    if (iter != componentCount.end())
    {
        return iter->second > 0;
    }
    else
    {
        return false;
    }
}

void StaticControlInformation::VisitParent(const Function<void(const ControlInformation*)>& visitor) const
{
    visitor(parent);
}

void StaticControlInformation::VisitChildren(const Function<void(const ControlInformation*)>& visitor) const
{
    for (const std::shared_ptr<ControlInformation>& child : children)
    {
        visitor(child.get());
    }
}

Any StaticControlInformation::GetControlPropertyValue(const DAVA::ReflectedStructure::Field& member) const
{
    Map<FastName, Any>::const_iterator iter = controlProperties.find(FastName(member.name));

    if (iter != controlProperties.end())
    {
        return iter->second;
    }
    else
    {
        return VariantType();
    }
}

StaticControlInformation* StaticControlInformation::GetParent() const
{
    return parent;
}

void StaticControlInformation::SetParent(StaticControlInformation* parent_)
{
    parent = parent_;
}

void StaticControlInformation::AddChild(const std::shared_ptr<StaticControlInformation>& child)
{
    children.push_back(child);
}

const Vector<std::shared_ptr<StaticControlInformation>>& StaticControlInformation::GetChildren() const
{
    return children;
}

std::shared_ptr<StaticControlInformation> StaticControlInformation::FindChildByName(const FastName& name) const
{
    for (const std::shared_ptr<StaticControlInformation>& c : children)
    {
        if (c->GetName() == name)
        {
            return c;
        }
    }

    return std::shared_ptr<StaticControlInformation>();
}

void StaticControlInformation::AddComponent(UIComponent::eType componentType)
{
    ++componentCount[componentType];
}

void StaticControlInformation::SetControlProperty(const DAVA::ReflectedStructure::Field& member, const DAVA::Any& value)
{
    controlProperties[FastName(member.name)] = value;
}

void StaticControlInformation::SetComponentProperty(UIComponent::eType componentType, int32 componentIndex, const DAVA::ReflectedStructure::Field& member, const DAVA::Any& value)
{
    const ComponentPropertyId id = std::make_tuple(componentType, componentIndex, FastName(member.name));
    componentProperties[id] = value;
}

void StaticControlInformation::AddResult(const DAVA::Result& result)
{
    results.AddResult(result);
}
