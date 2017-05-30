#pragma once

#include "Math/Vector.h"

namespace DAVA
{
class LayoutFormula;
class UIControl;

class UILayoutSystemListener
{
public:
    virtual ~UILayoutSystemListener() = default;

    virtual void OnControlLayouted(UIControl* control)
    {
    }
    virtual void OnFormulaProcessed(UIControl* control, Vector2::eAxis axis, const LayoutFormula* formula)
    {
    }
    virtual void OnFormulaRemoved(UIControl* control, Vector2::eAxis axis, const LayoutFormula* formula)
    {
    }
};
}
