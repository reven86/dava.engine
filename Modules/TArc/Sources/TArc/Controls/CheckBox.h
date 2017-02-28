#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <QCheckBox>

namespace DAVA
{
namespace TArc
{
class CheckBox final : public ControlProxyImpl<QCheckBox>
{
public:
    enum class Fields : uint32
    {
        Checked,
        IsReadOnly,
        TextHint,
        FieldCount
    };

    CheckBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    CheckBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedfields) override;

    void SetupControl();
    void StateChanged(int newState);

    enum class eContainedDataType : uint8
    {
        TYPE_NONE = 0,
        TYPE_BOOL,
        TYPE_CHECK_STATE
    };

    eContainedDataType dataType = eContainedDataType::TYPE_NONE;
    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA
