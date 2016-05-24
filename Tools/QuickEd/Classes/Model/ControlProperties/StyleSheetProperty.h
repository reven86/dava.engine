#ifndef __QUICKED_STYLE_SHEET_PROPERTY_H__
#define __QUICKED_STYLE_SHEET_PROPERTY_H__

#include "Model/ControlProperties/ValueProperty.h"
#include "UI/Styles/UIStyleSheetStructs.h"

class ValueProperty;
class IntrospectionProperty;

namespace DAVA
{
class UIControl;
}

class StyleSheetProperty : public ValueProperty
{
public:
    StyleSheetProperty(const DAVA::UIStyleSheetProperty& aProperty);

protected:
    virtual ~StyleSheetProperty();

public:
    void Accept(PropertyVisitor* visitor) override;

    DAVA::uint32 GetFlags() const override;

    DAVA::VariantType GetValue() const override;
    void ApplyValue(const DAVA::VariantType& value) override;

    DAVA::Interpolation::FuncType GetTransitionFunction() const;
    void SetTransitionFunction(DAVA::Interpolation::FuncType type);

    DAVA::int32 GetTransitionFunctionAsInt() const;
    void SetTransitionFunctionFromInt(DAVA::int32 type);

    DAVA::float32 GetTransitionTime() const;
    void SetTransitionTime(DAVA::float32 transitionTime);

    bool HasTransition() const;
    void SetTransition(bool transition);

    DAVA::uint32 GetPropertyIndex() const;
    const DAVA::UIStyleSheetProperty& GetProperty() const;

private:
    DAVA::UIStyleSheetProperty property;

public:
    INTROSPECTION_EXTEND(StyleSheetProperty, ValueProperty,
                         PROPERTY("transition", "Transition", HasTransition, SetTransition, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT)
                         PROPERTY("transitionTime", "Transition Time", GetTransitionTime, SetTransitionTime, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT)
                         PROPERTY("transitionFunction", DAVA::InspDesc("Transition Function", GlobalEnumMap<DAVA::Interpolation::FuncType>::Instance()), GetTransitionFunctionAsInt, SetTransitionFunctionFromInt, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT)
                         );
};

#endif // __QUICKED_STYLE_SHEET_PROPERTY_H__
