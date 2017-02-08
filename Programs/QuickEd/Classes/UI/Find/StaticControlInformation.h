#pragma once

#include "ControlInformation.h"

class StaticPackageInformation;

class StaticControlInformation
: public ControlInformation
{
public:
    StaticControlInformation(const DAVA::FastName& name);
    StaticControlInformation(const StaticControlInformation& other);
    StaticControlInformation(const StaticControlInformation& other, const DAVA::FastName& name, const std::shared_ptr<StaticPackageInformation> prototypePackage, const DAVA::FastName& prototype);

    DAVA::FastName GetName() const override;
    DAVA::FastName GetPrototype() const override;
    DAVA::String GetPrototypePackagePath() const override;

    void VisitParent(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;
    void VisitChildren(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;

    StaticControlInformation* GetParent() const;
    void SetParent(StaticControlInformation* parent);

    void AddChild(const std::shared_ptr<StaticControlInformation>& child);
    const DAVA::Vector<std::shared_ptr<StaticControlInformation>>& GetChildren() const;
    std::shared_ptr<StaticControlInformation> FindChildByName(const DAVA::FastName& name) const;

private:
    DAVA::FastName name;
    StaticControlInformation* parent = nullptr;

    std::shared_ptr<StaticPackageInformation> prototypePackage;
    DAVA::FastName prototype;

    DAVA::Vector<std::shared_ptr<StaticControlInformation>> children;
};
