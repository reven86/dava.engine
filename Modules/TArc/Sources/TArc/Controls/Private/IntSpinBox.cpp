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
    : TBase(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
}

IntSpinBox::IntSpinBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : TBase(ControlDescriptor(fields), accessor, model, parent)
{
}

bool IntSpinBox::FromText(const QString& input, int& output) const
{
    bool isOk = false;
    output = input.toInt(&isOk);
    return isOk;
}

QString IntSpinBox::ToText(const int output) const
{
    return QString::number(output);
}

bool IntSpinBox::IsEqualValue(int v1, int v2) const
{
    return v1 == v2;
}

QValidator::State IntSpinBox::TypeSpecificValidate(const QString& input) const
{
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

    return QValidator::Acceptable;
}

} // namespace TArc
} // namespace DAVA