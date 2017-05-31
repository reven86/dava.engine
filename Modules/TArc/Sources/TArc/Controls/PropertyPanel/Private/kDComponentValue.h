#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/MultiDoubleSpinBox.h"

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>
#include <Base/Vector.h>
#include <Base/Array.h>

namespace DAVA
{
namespace TArc
{
template <typename T, typename TEditor, typename TComponent>
class kDComponentValue : public BaseComponentValue
{
public:
    kDComponentValue();

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override;

private:
    Any Get1Axis() const;
    Any Get2Axis() const;
    Any Get3Axis() const;
    Any Get4Axis() const;
    Any Get5Axis() const;
    Any Get6Axis() const;
    void Set1Axis(const Any& v);
    void Set2Axis(const Any& v);
    void Set3Axis(const Any& v);
    void Set4Axis(const Any& v);
    void Set5Axis(const Any& v);
    void Set6Axis(const Any& v);

    Any GetFullValue() const;
    void SetFullValue(const Any& v);

    int32 GetAccuracy() const;
    const M::Range* Get1AxisRange() const;
    const M::Range* Get2AxisRange() const;
    const M::Range* Get3AxisRange() const;
    const M::Range* Get4AxisRange() const;
    const M::Range* Get5AxisRange() const;
    const M::Range* Get6AxisRange() const;

    Array<std::unique_ptr<M::Range>, 6> ranges;
    Vector<typename TEditor::FieldDescriptor> fields;

    DAVA_VIRTUAL_REFLECTION(kDComponentValue, BaseComponentValue);
};

} // namespace TArc
} // namespace DAVA