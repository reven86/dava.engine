#include "TArc/Controls/DoubleSpinBox.h"

namespace DAVA
{
namespace TArc
{
DoubleSpinBox::DoubleSpinBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    setDecimals(6);
}

DoubleSpinBox::DoubleSpinBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    setDecimals(6);
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
    if (input == ".")
    {
        output = 0.0;
        return true;
    }
    bool isOk = false;
    output = input.toDouble(&isOk);
    return isOk;
}

QString DoubleSpinBox::ToText(const double value) const
{
    return QString::number(value, 'f');
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

        int inputSize = input.size();
        if (inputSize == 1)
        {
            return QValidator::Intermediate;
        }
    }

    return QValidator::Acceptable;
}

QSize DoubleSpinBox::sizeHint() const
{
    QSize s = TBase::sizeHint();
    if (decimals() > 3)
    {
        s.setWidth(s.width() >> 1);
    }

    return s;
}

QSize DoubleSpinBox::minimumSizeHint() const
{
    QSize s = TBase::minimumSizeHint();
    if (decimals() > 3)
    {
        s.setWidth(s.width() >> 1);
    }

    return s;
}

} // namespace TArc
} // namespace DAVA
