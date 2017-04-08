#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Controls/Private/ValidatorDelegate.h"

#include <Base/BaseTypes.h>
#include <Base/FastName.h>

#include <QLineEdit>
#include <QFlags>

namespace DAVA
{
namespace TArc
{
class LineEdit final : public ControlProxyImpl<QLineEdit>, private ValidatorDelegate
{
public:
    enum class Fields : uint32
    {
        Text,
        PlaceHolder,
        IsReadOnly,
        IsEnabled,
        FieldCount
    };

    LineEdit(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    LineEdit(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedFields) override;

    void SetupControl();
    void EditingFinished();

    M::ValidationResult Validate(const Any& value) const override;
    void ShowHint(const QString& message) override;

private:
    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA