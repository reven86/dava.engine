#pragma once

#include "ControlInformation.h"
#include "Model/PackageHierarchy/ControlNode.h"

class ControlNodeInformation
: public ControlInformation
{
public:
    ControlNodeInformation(const ControlNode* controlNode);

    DAVA::FastName GetName() const override;
    DAVA::FastName GetPrototype() const override;
    DAVA::String GetPrototypePackagePath() const override;

    void VisitParent(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;
    void VisitChildren(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;

private:
    const ControlNode* controlNode;
};
