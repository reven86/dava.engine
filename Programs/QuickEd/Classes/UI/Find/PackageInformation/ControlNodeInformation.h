#pragma once

#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/Find/PackageInformation/ControlInformation.h"

class ControlNodeInformation
: public ControlInformation
{
public:
    ControlNodeInformation(const ControlNode* controlNode);

    DAVA::FastName GetName() const override;
    DAVA::FastName GetPrototype() const override;
    DAVA::String GetPrototypePackagePath() const override;
    bool HasErrors() const override;

    bool HasComponent(DAVA::UIComponent::eType componentType) const override;

    void VisitParent(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;
    void VisitChildren(const DAVA::Function<void(const ControlInformation*)>& visitor) const override;

    DAVA::Any GetControlPropertyValue(const DAVA::ReflectedStructure::Field& member) const override;

private:
    const ControlNode* controlNode;
};
