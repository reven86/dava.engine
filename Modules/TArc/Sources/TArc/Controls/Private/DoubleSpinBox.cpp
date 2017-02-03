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
    return QValidator::Acceptable;
}

} // namespace TArc
} // namespace DAVA
