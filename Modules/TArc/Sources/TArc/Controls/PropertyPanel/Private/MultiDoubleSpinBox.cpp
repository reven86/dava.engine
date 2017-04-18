#include "TArc/Controls/PropertyPanel/Private/MultiDoubleSpinBox.h"
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
QWidget* CreatedEditor(const Reflection& r, const MultiDoubleSpinBox::FieldDescriptor& fieldDescr, QWidget* parent, T* accessor, const Reflection& model, Vector<ControlProxy*>& subControls)
{
    QWidget* result = new QWidget(parent);
    QtHBoxLayout* layout = new QtHBoxLayout(result);
    layout->setMargin(2);
    layout->setSpacing(2);

    QLabel* label = new QLabel(QString::fromStdString(fieldDescr.valueRole + ":"), result);
    layout->addWidget(label);

    ControlDescriptorBuilder<DoubleSpinBox::Fields> descr;
    descr[DoubleSpinBox::Fields::Value] = fieldDescr.valueRole;
    ApplyRole(descr, DoubleSpinBox::Fields::Range, fieldDescr.rangeRole);
    ApplyRole(descr, DoubleSpinBox::Fields::IsReadOnly, fieldDescr.readOnlyRole);
    ApplyRole(descr, DoubleSpinBox::Fields::Accuracy, fieldDescr.accuracyRole);
    DoubleSpinBox* spinBox = new DoubleSpinBox(descr, accessor, model, result);
    subControls.push_back(spinBox);
    layout->AddControl(spinBox);
    spinBox->ToWidgetCast()->installEventFilter(parent);

    QWidget* editor = spinBox->ToWidgetCast();
    QSizePolicy policy = editor->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    editor->setSizePolicy(policy);

    return result;
}
}

MultiDoubleSpinBox::MultiDoubleSpinBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
    SetupControl(wrappersProcessor);
}

MultiDoubleSpinBox::MultiDoubleSpinBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(ControlDescriptor(fields), accessor, model, parent)
{
    SetupControl(accessor);
}

void MultiDoubleSpinBox::focusInEvent(QFocusEvent* e)
{
    if (lastFocusedItem != nullptr)
    {
        lastFocusedItem->setFocus(e->reason());
    }
}

bool MultiDoubleSpinBox::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::FocusIn)
    {
        QWidget* widget = qobject_cast<QWidget*>(obj);
        if (widget != nullptr)
        {
            lastFocusedItem = widget;
        }
    }

    return false;
}

template <typename T>
void MultiDoubleSpinBox::SetupControl(T* accessor)
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
        layout->addWidget(MultiFieldsControlDetails::CreatedEditor(editableField, fieldDescr, this, accessor, model, subControls));
    }
}

void MultiDoubleSpinBox::ForceUpdate()
{
    for_each(subControls.begin(), subControls.end(), [](ControlProxy* proxy)
             {
                 proxy->ForceUpdate();
             });

    TBase::ForceUpdate();
}

void MultiDoubleSpinBox::TearDown()
{
    for_each(subControls.begin(), subControls.end(), [](ControlProxy* proxy)
             {
                 proxy->TearDown();
             });

    TBase::TearDown();
}

void MultiDoubleSpinBox::UpdateControl(const ControlDescriptor& descriptor)
{
}

bool MultiDoubleSpinBox::FieldDescriptor::operator==(const FieldDescriptor& other) const
{
    return valueRole == other.valueRole &&
    readOnlyRole == other.readOnlyRole &&
    accuracyRole == other.accuracyRole &&
    rangeRole == other.rangeRole;
}

} // namespace TArc
} // namespace DAVA