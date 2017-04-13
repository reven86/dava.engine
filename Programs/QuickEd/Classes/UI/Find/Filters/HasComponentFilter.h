#pragma once

#include "UI/Find/Filters/FindFilter.h"
#include <Base/BaseTypes.h>
#include <UI/Components/UIComponent.h>

class HasComponentFilter : public FindFilter
{
public:
    HasComponentFilter(DAVA::UIComponent::eType componentType);

    bool CanAcceptPackage(const PackageInformation* package) const override;
    bool CanAcceptControl(const ControlInformation* control) const override;

private:
    DAVA::UIComponent::eType requiredComponentType;
};
