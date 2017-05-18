#include "TArc/Controls/Private/BaseSpinBox.h"

#include <QtEvents>

namespace DAVA
{
namespace TArc
{
namespace BaseSpinBoxDetail
{
template <typename T>
std::pair<const Type*, std::pair<Any, Any>> CreateRangePair()
{
    return std::make_pair(Type::Instance<T>(),
                          std::make_pair(Any(std::numeric_limits<T>::lowest()), Any(std::numeric_limits<T>::max())));
}
}
template <typename TBase, typename TEditableType>
BaseSpinBox<TBase, TEditableType>::BaseSpinBox(const BaseParams& params, const ControlDescriptor& descriptor, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<TBase>(params, descriptor, wrappersProcessor, model, parent)
{
    static_assert(std::is_base_of<QAbstractSpinBox, TBase>::value, "TBase should be derived from QAbstractSpinBox");
    SetupSpinBoxBase();
}

template <typename TBase, typename TEditableType>
BaseSpinBox<TBase, TEditableType>::BaseSpinBox(const BaseParams& params, const ControlDescriptor& descriptor, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<TBase>(params, descriptor, accessor, model, parent)
{
    static_assert(std::is_base_of<QAbstractSpinBox, TBase>::value, "TBase should be derived from QAbstractSpinBox");
    SetupSpinBoxBase();
}

template <typename TBase, typename TEditableType>
bool BaseSpinBox<TBase, TEditableType>::event(QEvent* e)
{
    if (e->type() == QEvent::Wheel)
    {
        if (this->hasFocus() == false)
        {
            return false;
        }
    }

    return ControlProxyImpl<TBase>::event(e);
}

template <typename TBase, typename TEditableType>
void BaseSpinBox<TBase, TEditableType>::UpdateRange()
{
    using namespace BaseSpinBoxDetail;
    static Map<const Type*, std::pair<Any, Any>> defaultRangeMap
    {
      CreateRangePair<float32>(),
      CreateRangePair<float64>(),
      CreateRangePair<int8>(),
      CreateRangePair<uint8>(),
      CreateRangePair<int16>(),
      CreateRangePair<uint16>(),
      CreateRangePair<int32>(),
      CreateRangePair<uint32>(),
      // I don't create int64 and uint64 nodes because the most wide type that supported by QSpinBox is int
    };

    Reflection valueField = this->model.GetField(this->GetFieldName(BaseFields::Value));
    DVASSERT(valueField.IsValid());
    const Type* valueType = valueField.GetValueType();
    if (valueType->IsConst())
    {
        return;
    }

    auto iter = defaultRangeMap.find(valueType);

    TEditableType minV = std::numeric_limits<TEditableType>::lowest();
    TEditableType maxV = std::numeric_limits<TEditableType>::max();
    TEditableType valueStep = static_cast<TEditableType>(1);

    if (iter != defaultRangeMap.end()) // we can use any type that can be casted to double or int (Any for example)
    {
        minV = iter->second.first.Cast<TEditableType>(std::numeric_limits<TEditableType>::lowest());
        maxV = iter->second.second.Cast<TEditableType>(std::numeric_limits<TEditableType>::max());
    }

    const M::Range* rangeMeta = nullptr;
    FastName rangeFieldName = this->GetFieldName(BaseFields::Range);
    if (rangeFieldName.IsValid())
    {
        rangeMeta = this->template GetFieldValue<const M::Range*>(BaseFields::Range, nullptr);
    }

    if (rangeMeta == nullptr)
    {
        rangeMeta = valueField.GetMeta<M::Range>();
    }

    if (rangeMeta != nullptr)
    {
        minV = rangeMeta->minValue.Cast<TEditableType>(minV);
        maxV = rangeMeta->maxValue.Cast<TEditableType>(maxV);
        valueStep = rangeMeta->step.Cast<TEditableType>(valueStep);
    }

    if (minV != this->minimum() || maxV != this->maximum())
    {
        this->setRange(minV, maxV);
    }

    if (valueStep != this->singleStep())
    {
        this->setSingleStep(valueStep);
    }
}

template <typename TBase, typename TEditableType>
void BaseSpinBox<TBase, TEditableType>::UpdateControl(const ControlDescriptor& changedFields)
{
    UpdateRange();
    bool valueChanged = changedFields.IsChanged(BaseFields::Value);
    bool readOnlychanged = changedFields.IsChanged(BaseFields::IsReadOnly);
    if (valueChanged == true || readOnlychanged == true)
    {
        DAVA::Reflection fieldValue = this->model.GetField(changedFields.GetName(BaseFields::Value));
        DVASSERT(fieldValue.IsValid());

        this->setReadOnly(this->IsValueReadOnly(changedFields, BaseFields::Value, BaseFields::IsReadOnly));
        if (valueChanged == true)
        {
            DAVA::Any value = fieldValue.GetValue();
            if (value.CanCast<TEditableType>())
            {
                if (stateHistory.top() == ControlState::InvalidValue)
                {
                    ToValidState();
                }
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
}

template <typename TBase, typename TEditableType>
void BaseSpinBox<TBase, TEditableType>::SetupSpinBoxBase()
{
    this->setKeyboardTracking(false);
    connections.AddConnection(this, static_cast<void (TBase::*)(TEditableType)>(&TBase::valueChanged), DAVA::MakeFunction(this, &BaseSpinBox<TBase, TEditableType>::ValueChanged));
    ToValidState();

    this->setFocusPolicy(Qt::StrongFocus);
}

template <typename TBase, typename TEditableType>
void BaseSpinBox<TBase, TEditableType>::ValueChanged(TEditableType val)
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
                Any currentValue = this->wrapper.GetFieldValue(this->GetFieldName(BaseFields::Value));
                if (currentValue.CanCast<TEditableType>() == false || currentValue.Cast<TEditableType>() != val)
                {
                    ToValidState();
                    this->wrapper.SetFieldValue(this->GetFieldName(BaseFields::Value), val);
                    UpdateRange();
                    if (this->hasFocus() == true)
                    {
                        ToEditingState();
                    }
                }
            }
        }
    }
}

template <typename TBase, typename TEditableType>
void BaseSpinBox<TBase, TEditableType>::ToEditingState()
{
    DVASSERT(this->hasFocus() == true);
    if (stateHistory.top() == ControlState::Editing)
    {
        return;
    }

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
void BaseSpinBox<TBase, TEditableType>::ToInvalidState()
{
    stateHistory = Stack<ControlState>();
    stateHistory.push(ControlState::InvalidValue);
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    this->lineEdit()->setText(textFromValue(0));
}

template <typename TBase, typename TEditableType>
void BaseSpinBox<TBase, TEditableType>::ToValidState()
{
    stateHistory = Stack<ControlState>();
    stateHistory.push(ControlState::ValidValue);
    this->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
}

template <typename TBase, typename TEditableType>
void BaseSpinBox<TBase, TEditableType>::fixup(QString& str) const
{
    TEditableType v;
    if (FromText(str, v))
    {
        if (v < this->minimum() || v > this->maximum())
        {
            QString message = QString("Out of bounds %1 : %2").arg(this->minimum()).arg(this->maximum());
            NotificationParams notifParams;
            notifParams.title = "Invalid value";
            notifParams.message.message = message.toStdString();
            notifParams.message.type = ::DAVA::Result::RESULT_ERROR;
            this->controlParams.ui->ShowNotification(this->controlParams.wndKey, notifParams);
        }
    }
}

template <typename TBase, typename TEditableType>
QValidator::State BaseSpinBox<TBase, TEditableType>::validate(QString& input, int& /*pos*/) const
{
    RETURN_IF_MODEL_LOST(QValidator::Invalid);
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
                    NotificationParams notifParams;
                    notifParams.title = "Invalid value";
                    notifParams.message.message = r.message;
                    notifParams.message.type = ::DAVA::Result::RESULT_ERROR;
                    this->controlParams.ui->ShowNotification(this->controlParams.wndKey, notifParams);
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
QString BaseSpinBox<TBase, TEditableType>::textFromValue(TEditableType val) const
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
            result = ToText(val);
        }
    }
    break;
    default:
        break;
    }

    return result;
}

template <typename TBase, typename TEditableType>
TEditableType BaseSpinBox<TBase, TEditableType>::valueFromText(const QString& text) const
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
void BaseSpinBox<TBase, TEditableType>::focusInEvent(QFocusEvent* event)
{
    ControlProxyImpl<TBase>::focusInEvent(event);
    ToEditingState();
}

template <typename TBase, typename TEditableType>
void BaseSpinBox<TBase, TEditableType>::focusOutEvent(QFocusEvent* event)
{
    ControlProxyImpl<TBase>::focusOutEvent(event);
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