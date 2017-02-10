#include "TArc/Controls/Private/BaseSpinBox.h"

namespace DAVA
{
namespace TArc
{
template <typename TBase, typename TEditableType>
DAVA::TArc::BaseSpinBox<TBase, TEditableType>::BaseSpinBox(const ControlDescriptor& descriptor, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxy<TBase>(descriptor, wrappersProcessor, model, parent)
{
    static_assert(std::is_base_of<QAbstractSpinBox, TBase>::value, "TBase should be derived from QAbstractSpinBox");
    SetupSpinBoxBase();
}

template <typename TBase, typename TEditableType>
DAVA::TArc::BaseSpinBox<TBase, TEditableType>::BaseSpinBox(const ControlDescriptor& descriptor, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxy<TBase>(descriptor, accessor, model, parent)
{
    static_assert(std::is_base_of<QAbstractSpinBox, TBase>::value, "TBase should be derived from QAbstractSpinBox");
    SetupSpinBoxBase();
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::UpdateControl(const ControlDescriptor& changedFields)
{
    auto updateRangeFn = [this](const M::Range* range)
    {
        if (range == nullptr)
        {
            return;
        }

        TEditableType minV = range->minValue.Cast<TEditableType>(std::numeric_limits<TEditableType>::min());
        TEditableType maxV = range->maxValue.Cast<TEditableType>(std::numeric_limits<TEditableType>::max());
        if (minV != this->minimum() || maxV != this->maximum())
        {
            this->setRange(minV, maxV);
        }

        TEditableType valueStep = range->step.Cast<TEditableType>(1);
        if (valueStep != this->singleStep())
        {
            this->setSingleStep(valueStep);
        }
    };

    bool valueChanged = changedFields.IsChanged(BaseFields::Value);
    bool readOnlychanged = changedFields.IsChanged(BaseFields::IsReadOnly);
    if (valueChanged == true || readOnlychanged == true)
    {
        DAVA::Reflection fieldValue = this->model.GetField(changedFields.GetName(BaseFields::Value));
        DVASSERT(fieldValue.IsValid());

        this->setReadOnly(this->IsValueReadOnly(changedFields, BaseFields::Value, BaseFields::IsReadOnly));
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
                this->setValue(v);
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
        this->setEnabled(this->template GetFieldValue<bool>(BaseFields::IsEnabled, true));
    }

    if (changedFields.IsChanged(BaseFields::Range))
    {
        updateRangeFn(this->template GetFieldValue<const M::Range*, int>(BaseFields::Range, nullptr));
    }
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::SetupSpinBoxBase()
{
    this->setKeyboardTracking(false);
    connections.AddConnection(this, static_cast<void (TBase::*)(TEditableType)>(&TBase::valueChanged), DAVA::MakeFunction(this, &BaseSpinBox<TBase, TEditableType>::ValueChanged));
    ToValidState();
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::ValueChanged(TEditableType val)
{
    if (this->isReadOnly() == true || this->isEnabled() == false)
    {
        return;
    }

    ControlState currentState = stateHistory.top();
    if (currentState == ControlState::Editing)
    {
        QString text = this->lineEdit()->text();
        TEditableType inputValue;
        if (FromText(text, inputValue))
        {
            if (IsEqualValue(inputValue, val))
            {
                ToValidState();
                this->wrapper.SetFieldValue(this->GetFieldName(BaseFields::Value), val);
                if (this->hasFocus() == true)
                {
                    ToEditingState();
                }
            }
        }
    }
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::ToEditingState()
{
    DVASSERT(this->hasFocus() == true);
    DVASSERT(stateHistory.top() != ControlState::Editing);

    ControlState prevState = stateHistory.top();
    stateHistory.push(ControlState::Editing);
    if (prevState == ControlState::InvalidValue)
    {
        this->lineEdit()->setText("");
        this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    }
    else
    {
        this->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    }
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::ToInvalidState()
{
    stateHistory = Stack<ControlState>();
    stateHistory.push(ControlState::InvalidValue);
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    this->lineEdit()->setText(textFromValue(0));
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::ToValidState()
{
    stateHistory = Stack<ControlState>();
    stateHistory.push(ControlState::ValidValue);
    this->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::fixup(QString& str) const
{
    TEditableType v;
    if (FromText(str, v))
    {
        if (v < this->minimum() || v > this->maximum())
        {
            QString message = QString("Out of bounds %1 : %2").arg(this->minimum()).arg(this->maximum());
            QToolTip::showText(this->mapToGlobal(this->geometry().topLeft()), message);
        }
    }
}

template <typename TBase, typename TEditableType>
QValidator::State DAVA::TArc::BaseSpinBox<TBase, TEditableType>::validate(QString& input, int& /*pos*/) const
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
        if (this->minimum() <= v && v <= this->maximum())
        {
            result = QValidator::Acceptable;
            Reflection valueField = this->model.GetField(this->GetFieldName(BaseFields::Value));
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
                    QToolTip::showText(this->mapToGlobal(QPoint(0, 0)), QString::fromStdString(r.message));
                }

                result = ConvertValidationState(r.state);
            }
        }
        else
        {
            TEditableType zeroNearestBoundary = Min(Abs(this->minimum()), Abs(this->maximum()));
            if (Abs(v) < zeroNearestBoundary)
            {
                result = QValidator::Intermediate;
            }
        }
    }

    return result;
}

template <typename TBase, typename TEditableType>
QString DAVA::TArc::BaseSpinBox<TBase, TEditableType>::textFromValue(TEditableType val) const
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
            QString editText = this->lineEdit()->text();
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

template <typename TBase, typename TEditableType>
TEditableType DAVA::TArc::BaseSpinBox<TBase, TEditableType>::valueFromText(const QString& text) const
{
    if (stateHistory.top() == ControlState::InvalidValue)
    {
        return this->value();
    }

    TEditableType v = TEditableType();
    FromText(text, v);
    return v;
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::keyPressEvent(QKeyEvent* event)
{
    ControlProxy<TBase>::keyPressEvent(event);
    int key = event->key();
    if (key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        this->valueChanged(this->value());
    }
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::focusInEvent(QFocusEvent* event)
{
    ControlProxy<TBase>::focusInEvent(event);
    ToEditingState();
}

template <typename TBase, typename TEditableType>
void DAVA::TArc::BaseSpinBox<TBase, TEditableType>::focusOutEvent(QFocusEvent* event)
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

#if __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wweak-template-vtables\"")
#endif

template class BaseSpinBox<QSpinBox, int>;
template class BaseSpinBox<QDoubleSpinBox, double>;

#if __clang__
_Pragma("clang diagnostic pop")
#endif

} // namespace TArc
} // namespace DAVA