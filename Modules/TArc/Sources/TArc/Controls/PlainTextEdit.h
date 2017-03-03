#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <QPlainTextEdit>
#include <QFlags>

namespace DAVA
{
namespace TArc
{
class PlainTextEdit final : public ControlProxyImpl<QPlainTextEdit>
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

    PlainTextEdit(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    PlainTextEdit(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void keyPressEvent(QKeyEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;

    void insertFromMimeData(const QMimeData* source) override;

    void UpdateControl(const ControlDescriptor& changedFields) override;

private:
    int maxLength = 10;
};
} // namespace TArc
} // namespace DAVA
