#pragma once
#include "TArc/Controls/ControlProxy.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
namespace TArc
{
class MultiFieldsControl : public ControlProxyImpl<QWidget>
{
    using TBase = ControlProxyImpl<QWidget>;

public:
    struct FieldDescriptor
    {
        String valueRole;
        String readOnlyRole;
        String rangeRole;
        String accuracyRole;

        bool operator==(const FieldDescriptor& other) const;
    };

    enum class Fields : uint32
    {
        FieldsList,
        FieldCount
    };

    MultiFieldsControl(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    MultiFieldsControl(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    template <typename T>
    void SetupControl(T* accessor);

    void UpdateControl(const ControlDescriptor& descriptor) override
    {
    }
};
} // namespace TArc
} // namespace DAVA