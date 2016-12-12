#include "ControlInformation.h"

#include "PackageInformation.h"

using namespace DAVA;

ControlInformation::ControlInformation(const FastName& name_)
    : name(name_)
{
}

ControlInformation::ControlInformation(const FastName& name_, const std::shared_ptr<PackageInformation> prototypePackage_, const DAVA::FastName& prototype_)
    : name(name_)
    , prototypePackage(prototypePackage_)
    , prototype(prototype_)
{
}

const DAVA::FastName& ControlInformation::GetName() const
{
    return name;
}

ControlInformation* ControlInformation::GetParent() const
{
    return parent;
}

void ControlInformation::SetParent(ControlInformation* parent_)
{
    parent = parent_;
}

String ControlInformation::GetPathToControl() const
{
    String result(name.c_str());
    ControlInformation* p = parent;
    while (p != nullptr)
    {
        result = String(p->GetName().c_str()) + "/" + result;
        p = p->GetParent();
    }

    return result;
}

const DAVA::FastName& ControlInformation::GetPrototype() const
{
    return prototype;
}

const DAVA::String& ControlInformation::GetPrototypePackagePath() const
{
    return prototypePackage->GetPath();
}

void ControlInformation::AddChild(const std::shared_ptr<ControlInformation>& child)
{
    children.push_back(child);
}

const DAVA::Vector<std::shared_ptr<ControlInformation>>& ControlInformation::GetChildren() const
{
    return children;
}
