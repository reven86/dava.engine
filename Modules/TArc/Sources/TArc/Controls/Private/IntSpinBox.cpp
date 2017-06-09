#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/Private/ValidationUtils.h"

#include <Reflection/ReflectedMeta.h>

#include <QLineEdit>
#include <QtEvents>

namespace DAVA
{
namespace TArc
{
IntSpinBox::IntSpinBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
}

IntSpinBox::IntSpinBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), accessor, model, parent)
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
    }

    return QValidator::Acceptable;
}

} // namespace TArc
} // namespace DAVA