#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/MultiDoubleSpinBox.h"

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace TArc
{
template <typename T>
class VectorComponentValue : public BaseComponentValue
{
public:
    VectorComponentValue() = default;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const override;

private:
    Vector<MultiDoubleSpinBox::FieldDescriptor> GetFieldsList() const;
    Any GetX() const;
    Any GetY() const;
    Any GetZ() const;
    Any GetW() const;
    void SetX(const Any& x);
    void SetY(const Any& y);
    void SetZ(const Any& x);
    void SetW(const Any& y);

    int32 GetAccuracy() const;
    const M::Range* GetXRange() const;
    const M::Range* GetYRange() const;
    const M::Range* GetZRange() const;
    const M::Range* GetWRange() const;

    Array<std::unique_ptr<M::Range>, 4> ranges;
    Vector<MultiDoubleSpinBox::FieldDescriptor> fields;

    DAVA_VIRTUAL_REFLECTION(VectorComponentValue, BaseComponentValue);
};

} // namespace TArc
} // namespace DAVA