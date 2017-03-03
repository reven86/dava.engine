#include "StaticControlInformation.h"

#include "StaticPackageInformation.h"

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

bool StaticControlInformation::HasComponent(UIComponent::eType componentType) const
{
    return components.test(componentType);
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

void StaticControlInformation::AddComponent(DAVA::UIComponent::eType componentType)
{
    components.set(componentType);
}
