#pragma once

#include "TArc/Controls/ControlProxy.h"
#include <Base/Vector.h>

#include <QWidget>

namespace DAVA
{
namespace TArc
{
class SubPropertiesEditor : public ControlProxy<QWidget>
{
public:
    enum class Fields : uint32
    {
        Value,
        IsReadOnly,
        FieldCount
    };

    SubPropertiesEditor(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    SubPropertiesEditor(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

protected:
    void UpdateControl(const ControlDescriptor& descriptor) override;

    template <typename T>
    void SetupControl(T* accessor);
};

} // namespace TArc
} // namespace DAVA
