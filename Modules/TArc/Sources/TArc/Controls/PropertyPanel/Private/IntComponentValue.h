#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

namespace DAVA
{
namespace TArc
{
class IntComponentValue : public BaseComponentValue
{
public:

private:
    double GetValue() const;
    void SetValue(double v);

    int GetMinValue() const;
    int GetMaxValue() const;
    bool IsReadOnly() const;

private:
    DAVA_VIRTUAL_REFLECTION(IntComponentValue, BaseComponentValue);
};
}
}