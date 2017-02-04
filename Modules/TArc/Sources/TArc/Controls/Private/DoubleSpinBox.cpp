#include "TArc/Controls/DoubleSpinBox.h"

namespace DAVA
{
namespace TArc
{
DoubleSpinBox::DoubleSpinBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
}

DoubleSpinBox::DoubleSpinBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(ControlDescriptor(fields), accessor, model, parent)
{
}

void DoubleSpinBox::UpdateControl(const ControlDescriptor& changedFields)
{
    if (changedFields.IsChanged(Fields::Accuracy))
    {
        DAVA::Reflection r = model.GetField(changedFields.GetName(Fields::Accuracy));
        DVASSERT(r.IsValid());
        setDecimals(r.GetValue().Cast<int>());
    }
    else if (changedFields.IsChanged(Fields::Value))
    {
        DAVA::Reflection r = model.GetField(changedFields.GetName(Fields::Value));
        DVASSERT(r.IsValid());
        const M::FloatNumberAccuracy* meta = r.GetMeta<M::FloatNumberAccuracy>();
        if (meta != nullptr)
        {
            setDecimals(meta->accuracy);
        }
    }

    TBase::UpdateControl(changedFields);
}

bool DoubleSpinBox::FromText(const QString& input, double& output) const
{
    bool isOk = false;
    output = input.toDouble(&isOk);
    return isOk;
}

QString DoubleSpinBox::ToText(const double& value) const
{
    return QString::number(value, 'f', decimals());
}

bool DoubleSpinBox::IsEqualValue(double v1, double v2) const
{
    double diff = Abs(v1 - v2);
    double accuracy = std::pow(10.0, -static_cast<double>(decimals()));
    return diff < accuracy;
}

QValidator::State DoubleSpinBox::TypeSpecificValidate(const QString& input) const
{
    if (input[0] == QChar('-'))
    {
        if (minimum() >= 0)
        {
            return QValidator::Invalid;
        }

        if (input.size() < 3)
        {
            return QValidator::Intermediate;
        }

        if (input[1].digitValue() == 0 && input[2] != QChar('.'))
        {
            return QValidator::Invalid;
        }
    }
    else
    {
        if (input.size() >= 2 && input[0].digitValue() == 0 && input[1] != QChar('.'))
        {
            return QValidator::Invalid;
        }
    }

    int pointIndex = input.indexOf('.');
    if (pointIndex != -1 && (input.size() - pointIndex) > decimals() + 1)
    {
        return QValidator::Invalid;
    }

    return QValidator::Acceptable;
}

} // namespace TArc
} // namespace DAVA
