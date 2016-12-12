#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"

class PackageInformation;

class ControlInformation
{
public:
    ControlInformation(const DAVA::FastName& name);
    ControlInformation(const DAVA::FastName& name, const std::shared_ptr<PackageInformation> prototypePackage, const DAVA::FastName& prototype);

    const DAVA::FastName& GetName() const;

    ControlInformation* GetParent() const;
    void SetParent(ControlInformation* parent);

    DAVA::String GetPathToControl() const;

    const DAVA::FastName& GetPrototype() const;
    const DAVA::String& GetPrototypePackagePath() const;
    void AddChild(const std::shared_ptr<ControlInformation>& child);
    const DAVA::Vector<std::shared_ptr<ControlInformation>>& GetChildren() const;

private:
    DAVA::FastName name;
    ControlInformation* parent = nullptr;

    std::shared_ptr<PackageInformation> prototypePackage;
    DAVA::FastName prototype;

    DAVA::Vector<std::shared_ptr<ControlInformation>> children;
};
