#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Functional/Functional.h"

class ControlInformation;

namespace ControlInformationHelpers
{
DAVA::String GetPathToControl(const ControlInformation* provider);
};

class ControlInformation
{
public:
    virtual ~ControlInformation(){};

    virtual DAVA::FastName GetName() const = 0;
    virtual DAVA::FastName GetPrototype() const = 0;
    virtual DAVA::String GetPrototypePackagePath() const = 0;

    virtual void VisitParent(const DAVA::Function<void(const ControlInformation*)>& visitor) const = 0;
    virtual void VisitChildren(const DAVA::Function<void(const ControlInformation*)>& visitor) const = 0;
};
