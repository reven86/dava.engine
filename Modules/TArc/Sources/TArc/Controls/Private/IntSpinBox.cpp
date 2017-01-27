#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/Private/ValidationUtils.h"

#include <Reflection/ReflectedMeta.h>

#include <QLineEdit>
#include <QtEvents>
#include <QToolTip>

namespace DAVA
{
namespace TArc
{
IntSpinBox::IntSpinBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxy<QSpinBox>(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

IntSpinBox::IntSpinBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxy<QSpinBox>(ControlDescriptor(fields), accessor, model, parent)
{
    SetupControl();
}

IntSpinBox::~IntSpinBox() = default;

void IntSpinBox::SetupControl()
{
    setKeyboardTracking(false);

    connections.AddConnection(this, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), DAVA::MakeFunction(this, &IntSpinBox::ValueChanged));
    ToValidState();
    setRange(3, 100);
}

void IntSpinBox::ValueChanged(int i)
{
    ControlState currentState = stateHistory.top();
    if (currentState == ControlState::Editing)
    {
        QString text = lineEdit()->text();
        bool isOk = false;
        int parsedValue = text.toInt(&isOk);
        if (isOk == true && i == parsedValue)
        {
            ToValidState();
            wrapper.SetFieldValue(GetFieldName(Fields::Value), i);
            if (hasFocus() == true)
            {
                ToEditingState();
            }
        }
    }
}

void IntSpinBox::ToEditingState()
{
    DVASSERT(hasFocus() == true);
    DVASSERT(stateHistory.top() != ControlState::Editing);

    ControlState prevState = stateHistory.top();
    stateHistory.push(ControlState::Editing);
    if (prevState == ControlState::InvalidValue)
    {
        lineEdit()->setText("");
        setButtonSymbols(QAbstractSpinBox::NoButtons);
    }
    else
    {
        setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    }
}

void IntSpinBox::ToInvalidState()
{
    stateHistory = Stack<ControlState>();
    stateHistory.push(ControlState::InvalidValue);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    lineEdit()->setText(textFromValue(0));
}

void IntSpinBox::ToValidState()
{
    stateHistory = Stack<ControlState>();
    stateHistory.push(ControlState::ValidValue);
    setButtonSymbols(QAbstractSpinBox::UpDownArrows);
}

void IntSpinBox::UpdateControl(const ControlDescriptor& changedFields)
{
    auto updateRangeFn = [this](const M::Range* range)
    {
        if (range == nullptr)
        {
            return;
        }

        int minV = range->minValue.Cast<int>(std::numeric_limits<int>::min());
        int maxV = range->maxValue.Cast<int>(std::numeric_limits<int>::max());
        if (minV != minimum() || maxV != maximum())
        {
            setRange(minV, maxV);
        }

        int valueStep = range->step.Cast<int>(1);
        if (valueStep != singleStep())
        {
            setSingleStep(valueStep);
        }
    };

    bool valueChanged = changedFields.IsChanged(Fields::Value);
    bool readOnlychanged = changedFields.IsChanged(Fields::IsReadOnly);
    if (valueChanged == true || readOnlychanged == true)
    {
        DAVA::Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Value));
        DVASSERT(fieldValue.IsValid());

        bool isReadOnly = fieldValue.IsReadonly() || fieldValue.GetMeta<M::ReadOnly>();
        if (changedFields.IsChanged(Fields::IsReadOnly))
        {
            DAVA::Reflection readOnlyField = model.GetField(changedFields.GetName(Fields::IsReadOnly));
            DVASSERT(readOnlyField.IsValid());
            isReadOnly |= readOnlyField.GetValue().Cast<bool>();
        }

        setReadOnly(isReadOnly);
        if (changedFields.GetName(Fields::Range).IsValid() == false)
        {
            updateRangeFn(fieldValue.GetMeta<M::Range>());
        }

        if (valueChanged == true)
        {
            DAVA::Any value = fieldValue.GetValue();
            if (value.CanCast<int32>())
            {
                int v = value.Cast<int32>();
                setValue(v);
            }
            else
            {
                ToInvalidState();
            }
        }
    }

    if (changedFields.IsChanged(Fields::IsEnabled))
    {
        DAVA::Reflection enabledField = model.GetField(changedFields.GetName(Fields::IsEnabled));
        DVASSERT(enabledField.IsValid());
        setEnabled(enabledField.GetValue().Cast<bool>());
    }

    if (changedFields.IsChanged(Fields::Range))
    {
        DAVA::Reflection rangeField = model.GetField(changedFields.GetName(Fields::Range));
        DVASSERT(rangeField.IsValid());
        updateRangeFn(rangeField.GetValue().Cast<const M::Range*>());
    }
}

QString IntSpinBox::textFromValue(int val) const
{
    QString result;
    switch (stateHistory.top())
    {
    case DAVA::TArc::IntSpinBox::ControlState::ValidValue:
        result = QString::number(val);
        break;
    case DAVA::TArc::IntSpinBox::ControlState::InvalidValue:
        result = QString("<multiple values>");
        break;
    case DAVA::TArc::IntSpinBox::ControlState::Editing:
    {
        Stack<ControlState> stateHistoryCopy = stateHistory;
        stateHistoryCopy.pop();
        DVASSERT(stateHistoryCopy.empty() == false);
        bool convertValToString = stateHistoryCopy.top() == ControlState::ValidValue;
        if (convertValToString == false)
        {
            QString editText = lineEdit()->text();
            bool parseOk = false;
            int parsedValue = editText.toInt(&parseOk);
            if ((parseOk == true && val == parsedValue))
            {
                convertValToString = true;
            }
        }

        if (convertValToString == true)
        {
            result = QString::number(val);
        }
    }
    break;
    default:
        break;
    }

    return result;
}

int IntSpinBox::valueFromText(const QString& text) const
{
    if (stateHistory.top() == ControlState::InvalidValue)
    {
        return value();
    }

    return text.toInt();
}

void IntSpinBox::fixup(QString& str) const
{
    bool isOk = false;
    int v = str.toInt(&isOk);

    if (isOk && (v < minimum() || v > maximum()))
    {
        QString message = QString("Out of bounds %1 : %2").arg(minimum()).arg(maximum());
        QToolTip::showText(mapToGlobal(geometry().topLeft()), message);
    }
}

QValidator::State IntSpinBox::validate(QString& input, int& pos) const
{
    ControlState currentState = stateHistory.top();
    if (currentState == ControlState::InvalidValue || currentState == ControlState::ValidValue)
    {
        return QValidator::Acceptable;
    }

    if (input.isEmpty())
    {
        return QValidator::Intermediate;
    }

    if (input[0] == QChar('-'))
    {
        if (minimum() >= 0)
        {
            return QValidator::Invalid;
        }

        if (input.size() == 1)
        {
            return QValidator::Intermediate;
        }

        if (input[1].digitValue() == 0)
        {
            return QValidator::Invalid;
        }
    }
    else
    {
        if (input.size() >= 2 && input[1].digitValue() == 0)
        {
            return QValidator::Invalid;
        }
    }

    bool isOk = false;
    int v = input.toInt(&isOk);

    QValidator::State result = QValidator::Invalid;
    if (isOk)
    {
        if (minimum() <= v && v <= maximum())
        {
            result = QValidator::Acceptable;
            Reflection valueField = model.GetField(GetFieldName(Fields::Value));
            DVASSERT(valueField.IsValid());
            const M::Validator* validator = valueField.GetMeta<M::Validator>();
            if (validator != nullptr)
            {
                M::ValidationResult r = validator->Validate(v, valueField.GetValue());
                if (!r.fixedValue.IsEmpty())
                {
                    input = QString::number(r.fixedValue.Cast<int32>());
                }

                if (!r.message.empty())
                {
                    QToolTip::showText(mapToGlobal(geometry().topLeft()), QString::fromStdString(r.message));
                }

                result = ConvertValidationState(r.state);
            }
        }
        else
        {
            int32 zeroNearestBoundary = Min(Abs(minimum()), Abs(maximum()));
            if (Abs(v) < zeroNearestBoundary)
            {
                result = QValidator::Intermediate;
            }
        }
    }

    return result;
}

void IntSpinBox::keyPressEvent(QKeyEvent* event)
{
    ControlProxy<QSpinBox>::keyPressEvent(event);
    int key = event->key();
    if (key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        ValueChanged(value());
    }
}

void IntSpinBox::focusInEvent(QFocusEvent* event)
{
    ControlProxy<QSpinBox>::focusInEvent(event);
    ToEditingState();
}

void IntSpinBox::focusOutEvent(QFocusEvent* event)
{
    ControlProxy<QSpinBox>::focusOutEvent(event);
    if (stateHistory.top() == ControlState::Editing)
    {
        stateHistory.pop();
    }
    DVASSERT(stateHistory.empty() == false);
    DVASSERT(stateHistory.top() == ControlState::InvalidValue || stateHistory.top() == ControlState::ValidValue);

    if (stateHistory.top() == ControlState::ValidValue)
    {
        ToValidState();
    }
    else
    {
        ToInvalidState();
    }
}

} // namespace TArc
} // namespace DAVA