#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"

#include <QComboBox>

namespace DAVA
{
namespace TArc
{
class ComboBox final : public ControlProxyImpl<QComboBox>
{
public:
    enum class Fields : uint32
    {
        Value,
        Enumerator,
        IsReadOnly,
        FieldCount
    };

    ComboBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ComboBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void CurrentIndexChanged(int newCurrentItem);

    void SetupControl();
    void UpdateControl(const ControlDescriptor& changedfields) override;

    void CreateItems(const Reflection& fieldValue, const Reflection& fieldEnumerator);
    int SelectCurrentItem(const Reflection& fieldValue, const Reflection& fieldEnumerator);

    bool updateControlProceed = false;
    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA
