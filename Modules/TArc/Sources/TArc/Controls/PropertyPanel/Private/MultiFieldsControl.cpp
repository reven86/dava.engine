#include "TArc/Controls/PropertyPanel/Private/MultiFieldsControl.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/QtWrapLayout.h"
#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/DoubleSpinBox.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <Debug/DVAssert.h>

#include <QLabel>
#include <QWidget>

namespace DAVA
{
namespace TArc
{
namespace MultiFieldsControlDetails
{
template <typename TEnum>
void ApplyRole(ControlDescriptorBuilder<TEnum>& descriptor, TEnum role, const String& name)
{
    if (name.empty() == false)
    {
        descriptor[role] = name;
    }
}

template <typename T>
QWidget* CreatedEditor(const Reflection& r, const MultiFieldsControl::FieldDescriptor& fieldDescr, QWidget* parent, T* accessor, const Reflection& model)
{
    QWidget* result = new QWidget(parent);
    QtHBoxLayout* layout = new QtHBoxLayout(result);
    layout->setMargin(2);
    layout->setSpacing(2);

    QLabel* label = new QLabel(QString::fromStdString(fieldDescr.valueRole + ":"), result);
    layout->addWidget(label);

    QWidget* editor = nullptr;

    const Type* valueType = r.GetValueType();
    if (valueType == Type::Instance<float32>() ||
        valueType == Type::Instance<float64>())
    {
        ControlDescriptorBuilder<DoubleSpinBox::Fields> descr;
        descr[DoubleSpinBox::Fields::Value] = fieldDescr.valueRole;
        ApplyRole(descr, DoubleSpinBox::Fields::Range, fieldDescr.rangeRole);
        ApplyRole(descr, DoubleSpinBox::Fields::IsReadOnly, fieldDescr.readOnlyRole);
        ApplyRole(descr, DoubleSpinBox::Fields::Accuracy, fieldDescr.accuracyRole);
        DoubleSpinBox* spinBox = new DoubleSpinBox(descr, accessor, model, result);
        editor = spinBox->ToWidgetCast();
        layout->AddWidget(spinBox);
    }
    else if (valueType == Type::Instance<int32>() ||
             valueType == Type::Instance<uint32>() ||
             valueType == Type::Instance<int8>() ||
             valueType == Type::Instance<uint8>() ||
             valueType == Type::Instance<int16>() ||
             valueType == Type::Instance<uint16>())
    {
        ControlDescriptorBuilder<IntSpinBox::Fields> descr;
        descr[IntSpinBox::Fields::Value] = fieldDescr.valueRole;
        ApplyRole(descr, IntSpinBox::Fields::Range, fieldDescr.rangeRole);
        ApplyRole(descr, IntSpinBox::Fields::IsReadOnly, fieldDescr.readOnlyRole);
        IntSpinBox* spinBox = new IntSpinBox(descr, accessor, model, result);
        editor = spinBox->ToWidgetCast();
        layout->AddWidget(spinBox);
    }

    if (editor != nullptr)
    {
        QSizePolicy policy = editor->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Expanding);
        editor->setSizePolicy(policy);
    }

    return result;
}
}

MultiFieldsControl::MultiFieldsControl(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
    SetupControl(wrappersProcessor);
}

MultiFieldsControl::MultiFieldsControl(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(ControlDescriptor(fields), accessor, model, parent)
{
    SetupControl(accessor);
}

template <typename T>
void MultiFieldsControl::SetupControl(T* accessor)
{
    DVASSERT(layout() == nullptr);
    Reflection r = model.GetField(GetFieldName(Fields::FieldsList));
    DVASSERT(r.IsValid());

    Vector<Reflection::Field> fields = r.GetFields();
    if (fields.empty())
    {
        return;
    }

    QtWrapLayout* layout = new QtWrapLayout(this);
    layout->setMargin(2);
    layout->SetHorizontalSpacing(2);
    layout->SetVerticalSpacing(2);
    for (Reflection::Field& f : fields)
    {
        Any fieldValue = f.ref.GetValue();
        DVASSERT(fieldValue.CanGet<FieldDescriptor>());
        FieldDescriptor fieldDescr = fieldValue.Get<FieldDescriptor>();

        Reflection editableField = model.GetField(fieldDescr.valueRole);
        DVASSERT(editableField.IsValid());
        layout->addWidget(MultiFieldsControlDetails::CreatedEditor(editableField, fieldDescr, this, accessor, model));
    }
}

bool MultiFieldsControl::FieldDescriptor::operator==(const FieldDescriptor& other) const
{
    return valueRole == other.valueRole &&
    readOnlyRole == other.readOnlyRole &&
    accuracyRole == other.accuracyRole &&
    rangeRole == other.rangeRole;
}

} // namespace TArc
} // namespace DAVA