#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/Private/ValidationUtils.h"
#include "TArc/Utils/QtConnections.h"

#include <QLineEdit>
#include <QAbstractSpinBox>
#include <QString>
#include <QToolTip>
#include <QKeyEvent>

namespace DAVA
{
namespace TArc
{
template <typename TBase, typename TEditableType>
class BaseSpinBox : public ControlProxy<TBase>
{
public:
    enum BaseFields : uint32
    {
        Value,
        IsReadOnly,
        IsEnabled,
        Range
    };

    BaseSpinBox(const ControlDescriptor& descriptor, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
        : ControlProxy<TBase>(descriptor, wrappersProcessor, model, parent)
    {
        static_assert(std::is_base_of<QAbstractSpinBox, TBase>::value, "TBase should be derived from QAbstractSpinBox");
        SetupSpinBoxBase();
    }

    BaseSpinBox(const ControlDescriptor& descriptor, ContextAccessor* accessor, Reflection model, QWidget* parent)
        : ControlProxy<TBase>(descriptor, accessor, model, parent)
    {
        static_assert(std::is_base_of<QAbstractSpinBox, TBase>::value, "TBase should be derived from QAbstractSpinBox");
        SetupSpinBoxBase();
    }

protected:
    void UpdateControl(const ControlDescriptor& changedFields) override
    {
        auto updateRangeFn = [this](const M::Range* range)
        {
            if (range == nullptr)
            {
                return;
            }

            TEditableType minV = range->minValue.Cast<TEditableType>(std::numeric_limits<TEditableType>::min());
            TEditableType maxV = range->maxValue.Cast<TEditableType>(std::numeric_limits<TEditableType>::max());
            if (minV != minimum() || maxV != maximum())
            {
                setRange(minV, maxV);
            }

            TEditableType valueStep = range->step.Cast<TEditableType>(1);
            if (valueStep != singleStep())
            {
                setSingleStep(valueStep);
            }
        };

        bool valueChanged = changedFields.IsChanged(BaseFields::Value);
        bool readOnlychanged = changedFields.IsChanged(BaseFields::IsReadOnly);
        if (valueChanged == true || readOnlychanged == true)
        {
            DAVA::Reflection fieldValue = model.GetField(changedFields.GetName(BaseFields::Value));
            DVASSERT(fieldValue.IsValid());

            bool isReadOnly = fieldValue.IsReadonly() || fieldValue.GetMeta<M::ReadOnly>();
            if (changedFields.IsChanged(BaseFields::IsReadOnly))
            {
                DAVA::Reflection readOnlyField = model.GetField(changedFields.GetName(BaseFields::IsReadOnly));
                DVASSERT(readOnlyField.IsValid());
                isReadOnly |= readOnlyField.GetValue().Cast<bool>();
            }

            setReadOnly(isReadOnly);
            if (changedFields.GetName(BaseFields::Range).IsValid() == false)
            {
                updateRangeFn(fieldValue.GetMeta<M::Range>());
            }

            if (valueChanged == true)
            {
                DAVA::Any value = fieldValue.GetValue();
                if (value.CanCast<TEditableType>())
                {
                    TEditableType v = value.Cast<TEditableType>();
                    setValue(v);
                }
                else
                {
                    if (value.CanCast<String>())
                    {
                        noValueString = QString::fromStdString(value.Cast<String>());
                    }
                    ToInvalidState();
                }
            }
        }

        if (changedFields.IsChanged(BaseFields::IsEnabled))
        {
            DAVA::Reflection enabledField = model.GetField(changedFields.GetName(BaseFields::IsEnabled));
            DVASSERT(enabledField.IsValid());
            setEnabled(enabledField.GetValue().Cast<bool>());
        }

        if (changedFields.IsChanged(BaseFields::Range))
        {
            DAVA::Reflection rangeField = model.GetField(changedFields.GetName(BaseFields::Range));
            DVASSERT(rangeField.IsValid());
            updateRangeFn(rangeField.GetValue().Cast<const M::Range*>());
        }
    }

    void SetupSpinBoxBase()
    {
        setKeyboardTracking(false);
        connections.AddConnection(this, static_cast<void (TBase::*)(TEditableType)>(&TBase::valueChanged), DAVA::MakeFunction(this, &BaseSpinBox<TBase, TEditableType>::ValueChanged));
        ToValidState();
    }

    void ValueChanged(TEditableType val)
    {
        if (isReadOnly() == true || isEnabled() == false)
        {
            return;
        }

        ControlState currentState = stateHistory.top();
        if (currentState == ControlState::Editing)
        {
            QString text = lineEdit()->text();
            TEditableType inputValue;
            if (FromText(text, inputValue))
            {
                if (IsEqualValue(inputValue, val))
                {
                    ToValidState();
                    wrapper.SetFieldValue(GetFieldName(BaseFields::Value), val);
                    if (hasFocus() == true)
                    {
                        ToEditingState();
                    }
                }
            }
        }
    }

    void ToEditingState()
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

    void ToInvalidState()
    {
        stateHistory = Stack<ControlState>();
        stateHistory.push(ControlState::InvalidValue);
        setButtonSymbols(QAbstractSpinBox::NoButtons);
        lineEdit()->setText(textFromValue(0));
    }

    void ToValidState()
    {
        stateHistory = Stack<ControlState>();
        stateHistory.push(ControlState::ValidValue);
        setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    }

    virtual bool FromText(const QString& input, TEditableType& output) const = 0;
    virtual QString ToText(const TEditableType& output) const = 0;
    virtual bool IsEqualValue(TEditableType v1, TEditableType v2) const = 0;
    virtual QValidator::State TypeSpecificValidate(const QString& str) const = 0;

    void fixup(QString& str) const override
    {
        TEditableType v;
        if (FromText(str, v))
        {
            if (v < minimum() || v > maximum())
            {
                QString message = QString("Out of bounds %1 : %2").arg(minimum()).arg(maximum());
                QToolTip::showText(mapToGlobal(geometry().topLeft()), message);
            }
        }
    }

    QValidator::State validate(QString& input, int& pos) const override
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

        QValidator::State typeValidationState = TypeSpecificValidate(input);
        if (typeValidationState != QValidator::Acceptable)
        {
            return typeValidationState;
        }

        QValidator::State result = QValidator::Invalid;
        TEditableType v;

        if (FromText(input, v))
        {
            if (minimum() <= v && v <= maximum())
            {
                result = QValidator::Acceptable;
                Reflection valueField = model.GetField(GetFieldName(BaseFields::Value));
                DVASSERT(valueField.IsValid());
                const M::Validator* validator = valueField.GetMeta<M::Validator>();
                if (validator != nullptr)
                {
                    M::ValidationResult r = validator->Validate(v, valueField.GetValue());
                    if (!r.fixedValue.IsEmpty())
                    {
                        input = ToText(r.fixedValue.Cast<TEditableType>());
                    }

                    if (!r.message.empty())
                    {
                        QToolTip::showText(mapToGlobal(QPoint(0, 0)), QString::fromStdString(r.message));
                    }

                    result = ConvertValidationState(r.state);
                }
            }
            else
            {
                TEditableType zeroNearestBoundary = Min(Abs(minimum()), Abs(maximum()));
                if (Abs(v) < zeroNearestBoundary)
                {
                    result = QValidator::Intermediate;
                }
            }
        }

        return result;
    }

protected:
    QtConnections connections;
    QString noValueString = QStringLiteral("<multiple values>");

    enum class ControlState
    {
        ValidValue,
        InvalidValue,
        Editing
    };

    Stack<ControlState> stateHistory;

private:
    QString textFromValue(TEditableType val) const override
    {
        QString result;
        switch (stateHistory.top())
        {
        case ControlState::ValidValue:
            result = ToText(val);
            break;
        case ControlState::InvalidValue:
            result = noValueString;
            break;
        case ControlState::Editing:
        {
            Stack<ControlState> stateHistoryCopy = stateHistory;
            stateHistoryCopy.pop();
            DVASSERT(stateHistoryCopy.empty() == false);
            bool convertValToString = stateHistoryCopy.top() == ControlState::ValidValue;
            if (convertValToString == false)
            {
                QString editText = lineEdit()->text();
                TEditableType parsedValue;
                if (FromText(editText, parsedValue))
                {
                    convertValToString = IsEqualValue(val, parsedValue);
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

    TEditableType valueFromText(const QString& text) const override
    {
        if (stateHistory.top() == ControlState::InvalidValue)
        {
            return value();
        }

        TEditableType v = TEditableType();
        FromText(text, v);
        return v;
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        ControlProxy<TBase>::keyPressEvent(event);
        int key = event->key();
        if (key == Qt::Key_Enter || key == Qt::Key_Return)
        {
            emit valueChanged(value());
        }
    }

    void focusInEvent(QFocusEvent* event) override
    {
        ControlProxy<TBase>::focusInEvent(event);
        ToEditingState();
    }

    void focusOutEvent(QFocusEvent* event) override
    {
        ControlProxy<TBase>::focusOutEvent(event);
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
};

} // namespace TArc
} // namespace DAVA