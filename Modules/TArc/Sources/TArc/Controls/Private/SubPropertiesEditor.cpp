#include "TArc/Controls/SubPropertiesEditor.h"
#include "TArc/Controls/DoubleSpinBox.h"
#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/QtWrapLayout.h"
#include "TArc/Controls/QtBoxLayouts.h"

#include <QLabel>

namespace DAVA
{
namespace TArc
{
namespace SubPropertiesEditorDetail
{
template <typename TControl, typename TAccessor>
TControl* CreateControl(typename TControl::Fields role, const String& fieldName, TAccessor* accessor, const Reflection& model)
{
    ControlDescriptorBuilder<typename TControl::Fields> descr;
    descr[role] = fieldName;
    TControl* control = new TControl(descr, accessor, model);
    return control;
}
}

SubPropertiesEditor::SubPropertiesEditor(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(fields, wrappersProcessor, model, parent)
{
    SetupControl(wrappersProcessor);
    copyModelWrapper = wrappersProcessor->CreateWrapper(MakeFunction(this, &SubPropertiesEditor::GetCopyModel), nullptr);
    copyModelWrapper.SetListener(this);
}

SubPropertiesEditor::SubPropertiesEditor(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(fields, accessor, model, parent)
{
    SetupControl(accessor);
    copyModelWrapper = accessor->CreateWrapper(MakeFunction(this, &SubPropertiesEditor::GetCopyModel));
    copyModelWrapper.SetListener(this);
}

void SubPropertiesEditor::UpdateControl(const ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::Value))
    {
        copyModel.SetValueWithCast(model.GetValue());
    }

    setEnabled(!IsValueReadOnly(descriptor, Fields::Value, Fields::IsReadOnly));
}

void SubPropertiesEditor::OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields)
{
    if (wrapper == copyModelWrapper)
    {
        this->wrapper.SetFieldValue(GetFieldName(Fields::Value), valueCopy);
        return;
    }

    ControlProxyImpl<QWidget>::OnDataChanged(wrapper, fields);
}

DAVA::Reflection SubPropertiesEditor::GetCopyModel(const DataContext* /*ctx*/)
{
    return copyModel;
}

template <typename T>
void DAVA::TArc::SubPropertiesEditor::SetupControl(T* accessor)
{
    using namespace SubPropertiesEditorDetail;
    FastName fieldName = GetFieldName(Fields::Value);
    DVASSERT(fieldName.IsValid());
    Reflection valueField = this->model.GetField(fieldName);
    DVASSERT(valueField.IsValid());

    valueCopy = valueField.GetValue();
    copyModel = Reflection::Create(valueCopy);

    QtWrapLayout* layout = new QtWrapLayout(this);
    layout->SetHorizontalSpacing(2);
    layout->SetVerticalSpacing(2);
    layout->setMargin(1);

    Vector<Reflection::Field> subFields = copyModel.GetFields();
    for (Reflection::Field& field : subFields)
    {
        if (field.ref.HasMeta<M::SubProperty>())
        {
            String subFieldName = field.key.Cast<String>();
            QtHBoxLayout* subPropertyLayout = new QtHBoxLayout();
            subPropertyLayout->setMargin(1);
            subPropertyLayout->setSpacing(4);
            layout->AddLayout(subPropertyLayout);

            QLabel* label = new QLabel(QString::fromStdString(subFieldName));
            subPropertyLayout->addWidget(label);

            QWidget* editorWidget = nullptr;

            Any subPropertyValue = field.ref.GetValue();
            if (subPropertyValue.CanGet<float32>() || subPropertyValue.CanGet<float64>())
            {
                DoubleSpinBox* control = CreateControl<DoubleSpinBox>(DoubleSpinBox::Fields::Value, subFieldName, accessor, copyModel);
                editorWidget = control->ToWidgetCast();
                subPropertyLayout->AddWidget(control);
            }
            else if (subPropertyValue.CanCast<int32>())
            {
                IntSpinBox* control = CreateControl<IntSpinBox>(IntSpinBox::Fields::Value, subFieldName, accessor, copyModel);
                editorWidget = control->ToWidgetCast();
                subPropertyLayout->AddWidget(control);
            }
            else if (subPropertyValue.CanCast<String>())
            {
                LineEdit* control = CreateControl<LineEdit>(LineEdit::Fields::Text, subFieldName, accessor, copyModel);
                editorWidget = control->ToWidgetCast();
                subPropertyLayout->AddWidget(control);
            }

            if (editorWidget != nullptr)
            {
                QSizePolicy sizePolicy = editorWidget->sizePolicy();
                sizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
                editorWidget->setSizePolicy(sizePolicy);
            }
        }
    }
}

} // namespace TArc
} // namespace DAVA
