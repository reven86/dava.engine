#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <QLabel>

namespace DAVA
{
namespace TArc
{
class Label final : public ControlProxyImpl<QLabel>
{
public:
    enum class Fields : uint32
    {
        Text,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    Label(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    Label(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedFields) override;
};
} // namespace TArc
} // namespace DAVA
