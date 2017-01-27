#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/BaseTypes.h>

#include <QSpinBox>
#include <QString>
#include <QVariant>
#include <QValidator>

namespace DAVA
{
namespace TArc
{
class IntSpinBox : public ControlProxy<QSpinBox>
{
public:
    enum class Fields : uint32
    {
        Value, // [ReadOnly, Validator, Range]
        IsReadOnly,
        IsEnabled,
        Range, // Value should be castable to " const M::Range* "
        FieldCount
    };

    IntSpinBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    IntSpinBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);
    ~IntSpinBox() override;

private:
    void UpdateControl(const ControlDescriptor& changedFields) override;
    void SetupControl();

    // SpinBox handlers
    void ValueChanged(int i);

    // Update this control utils
    void ToEditingState();
    void ToInvalidState();
    void ToValidState();

private:
    // Qt reimplemented
    QString textFromValue(int val) const override;
    int valueFromText(const QString& text) const override;
    void fixup(QString& str) const override;
    QValidator::State validate(QString& input, int& pos) const override;

    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    QtConnections connections;

    enum class ControlState
    {
        ValidValue,
        InvalidValue,
        Editing
    };

    Stack<ControlState> stateHistory;
};
} // namespace TArc
} // namespace DAVA