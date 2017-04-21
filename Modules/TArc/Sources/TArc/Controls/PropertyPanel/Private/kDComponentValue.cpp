#include "TArc/Controls/PropertyPanel/Private/kDComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/kDComponentValueTraits.h"
#include "TArc/Controls/PropertyPanel/Private/MultiDoubleSpinBox.h"
#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/Widget.h"
#include "TArc/Controls/ColorPicker/ColorPickerButton.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Math/Vector.h>
#include <Base/BaseTypes.h>

#include <QHBoxLayout>

namespace DAVA
{
namespace TArc
{
template <typename T, typename TEditor, typename TComponent>
DAVA::TArc::kDComponentValue<T, TEditor, TComponent>::kDComponentValue()
{
    using namespace KDComponentValueTraits;
    InitFieldsList<T, TEditor>(fields);
    InitRanges<T>(nodes, ranges);
}

template <typename T, typename TEditor, typename TComponent>
Any kDComponentValue<T, TEditor, TComponent>::GetMultipleValue() const
{
    return Any();
}

template <typename T, typename TEditor, typename TComponent>
bool kDComponentValue<T, TEditor, TComponent>::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    DVASSERT(Type::Instance<T>() == Type::Instance<Color>());
    if (newValue.IsEmpty())
    {
        return false;
    }

    return newValue != currentValue;
}

template <typename T, typename TEditor, typename TComponent>
ControlProxy* kDComponentValue<T, TEditor, TComponent>::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const
{
    if (Type::Instance<T>() == Type::Instance<Color>())
    {
        Widget* w = new Widget(parent);
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setSpacing(2);
        layout->setMargin(0);
        w->SetLayout(layout);

        {
            ColorPickerButton::Params params;
            params.accessor = GetAccessor();
            params.ui = GetUI();
            params.wndKey = GetWindowKey();
            params.fields[ColorPickerButton::Fields::Color] = "value";
            params.fields[ColorPickerButton::Fields::IsReadOnly] = readOnlyFieldName;
            w->AddControl(new ColorPickerButton(params, wrappersProcessor, model, w->ToWidgetCast()));
        }

        {
            ControlDescriptorBuilder<typename TEditor::Fields> descr;
            descr[TEditor::Fields::FieldsList] = "fieldsList";
            w->AddControl(new TEditor(descr, wrappersProcessor, model, w->ToWidgetCast()));
        }

        return w;
    }

    ControlDescriptorBuilder<typename TEditor::Fields> descr;
    descr[TEditor::Fields::FieldsList] = "fieldsList";
    return new TEditor(descr, wrappersProcessor, model, parent);
}

template <typename T, typename TEditor, typename TComponent>
Any kDComponentValue<T, TEditor, TComponent>::Get1Axis() const
{
    using namespace KDComponentValueTraits;
    return GetNodesAxisValue<T, TComponent, 0>(nodes);
}

template <typename T, typename TEditor, typename TComponent>
Any kDComponentValue<T, TEditor, TComponent>::Get2Axis() const
{
    using namespace KDComponentValueTraits;
    return GetNodesAxisValue<T, TComponent, 1>(nodes);
}

template <typename T, typename TEditor, typename TComponent>
Any kDComponentValue<T, TEditor, TComponent>::Get3Axis() const
{
    using namespace KDComponentValueTraits;
    return GetNodesAxisValue<T, TComponent, 2>(nodes);
}

template <typename T, typename TEditor, typename TComponent>
Any kDComponentValue<T, TEditor, TComponent>::Get4Axis() const
{
    using namespace KDComponentValueTraits;
    return GetNodesAxisValue<T, TComponent, 3>(nodes);
}

template <typename T, typename TEditor, typename TComponent>
Any kDComponentValue<T, TEditor, TComponent>::Get5Axis() const
{
    using namespace KDComponentValueTraits;
    return GetNodesAxisValue<T, TComponent, 4>(nodes);
}

template <typename T, typename TEditor, typename TComponent>
Any kDComponentValue<T, TEditor, TComponent>::Get6Axis() const
{
    using namespace KDComponentValueTraits;
    return GetNodesAxisValue<T, TComponent, 5>(nodes);
}

template <typename T, typename TEditor, typename TComponent>
void kDComponentValue<T, TEditor, TComponent>::Set1Axis(const Any& v)
{
    using namespace KDComponentValueTraits;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetNodesAxisValue<T, TComponent, 0>(nodes, v.Cast<TComponent>(), cmdInterface);
}

template <typename T, typename TEditor, typename TComponent>
void kDComponentValue<T, TEditor, TComponent>::Set2Axis(const Any& v)
{
    using namespace KDComponentValueTraits;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetNodesAxisValue<T, TComponent, 1>(nodes, v.Cast<TComponent>(), cmdInterface);
}

template <typename T, typename TEditor, typename TComponent>
void kDComponentValue<T, TEditor, TComponent>::Set3Axis(const Any& v)
{
    using namespace KDComponentValueTraits;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetNodesAxisValue<T, TComponent, 2>(nodes, v.Cast<TComponent>(), cmdInterface);
}

template <typename T, typename TEditor, typename TComponent>
void kDComponentValue<T, TEditor, TComponent>::Set4Axis(const Any& v)
{
    using namespace KDComponentValueTraits;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetNodesAxisValue<T, TComponent, 3>(nodes, v.Cast<TComponent>(), cmdInterface);
}

template <typename T, typename TEditor, typename TComponent>
void kDComponentValue<T, TEditor, TComponent>::Set5Axis(const Any& v)
{
    using namespace KDComponentValueTraits;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetNodesAxisValue<T, TComponent, 4>(nodes, v.Cast<TComponent>(), cmdInterface);
}

template <typename T, typename TEditor, typename TComponent>
void kDComponentValue<T, TEditor, TComponent>::Set6Axis(const Any& v)
{
    using namespace KDComponentValueTraits;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetNodesAxisValue<T, TComponent, 5>(nodes, v.Cast<TComponent>(), cmdInterface);
}

template <typename T, typename TEditor, typename TComponent>
Any kDComponentValue<T, TEditor, TComponent>::GetFullValue() const
{
    return GetValue();
}

template <typename T, typename TEditor, typename TComponent>
void kDComponentValue<T, TEditor, TComponent>::SetFillValue(const Any& v)
{
    SetValue(v);
}

template <typename T, typename TEditor, typename TComponent>
int32 kDComponentValue<T, TEditor, TComponent>::GetAccuracy() const
{
    if (!nodes.empty())
    {
        const M::FloatNumberAccuracy* accuracy = nodes.front()->field.ref.template GetMeta<M::FloatNumberAccuracy>();
        if (accuracy != nullptr)
        {
            return accuracy->accuracy;
        }
    }

    return 6;
}

template <typename T, typename TEditor, typename TComponent>
const M::Range* kDComponentValue<T, TEditor, TComponent>::Get1AxisRange() const
{
    return ranges[0].get();
}

template <typename T, typename TEditor, typename TComponent>
const M::Range* kDComponentValue<T, TEditor, TComponent>::Get2AxisRange() const
{
    return ranges[1].get();
}

template <typename T, typename TEditor, typename TComponent>
const M::Range* kDComponentValue<T, TEditor, TComponent>::Get3AxisRange() const
{
    return ranges[2].get();
}

template <typename T, typename TEditor, typename TComponent>
const M::Range* kDComponentValue<T, TEditor, TComponent>::Get4AxisRange() const
{
    return ranges[3].get();
}

template <typename T, typename TEditor, typename TComponent>
const M::Range* kDComponentValue<T, TEditor, TComponent>::Get5AxisRange() const
{
    return ranges[4].get();
}

template <typename T, typename TEditor, typename TComponent>
const M::Range* kDComponentValue<T, TEditor, TComponent>::Get6AxisRange() const
{
    return ranges[5].get();
}

using Vector2ComponentValue = kDComponentValue<Vector2, MultiDoubleSpinBox, float32>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(Vector2ComponentValue)
{
    ReflectionRegistrator<Vector2ComponentValue>::Begin()
    .Field("fieldsList", &Vector2ComponentValue::fields)
    .Field("X", &Vector2ComponentValue::Get1Axis, &Vector2ComponentValue::Set1Axis)
    .Field("Y", &Vector2ComponentValue::Get2Axis, &Vector2ComponentValue::Set2Axis)
    .Field("accuracy", &Vector2ComponentValue::GetAccuracy, nullptr)
    .Field("xRange", &Vector2ComponentValue::Get1AxisRange, nullptr)
    .Field("yRange", &Vector2ComponentValue::Get2AxisRange, nullptr)
    .End();
}

using Vector3ComponentValue = kDComponentValue<Vector3, MultiDoubleSpinBox, float32>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(Vector3ComponentValue)
{
    ReflectionRegistrator<Vector3ComponentValue>::Begin()
    .Field("fieldsList", &Vector3ComponentValue::fields)
    .Field("X", &Vector3ComponentValue::Get1Axis, &Vector3ComponentValue::Set1Axis)
    .Field("Y", &Vector3ComponentValue::Get2Axis, &Vector3ComponentValue::Set2Axis)
    .Field("Z", &Vector3ComponentValue::Get3Axis, &Vector3ComponentValue::Set3Axis)
    .Field("accuracy", &Vector3ComponentValue::GetAccuracy, nullptr)
    .Field("xRange", &Vector3ComponentValue::Get1AxisRange, nullptr)
    .Field("yRange", &Vector3ComponentValue::Get2AxisRange, nullptr)
    .Field("zRange", &Vector3ComponentValue::Get3AxisRange, nullptr)
    .End();
}

using Vector4ComponentValue = kDComponentValue<Vector4, MultiDoubleSpinBox, float32>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(Vector4ComponentValue)
{
    ReflectionRegistrator<Vector4ComponentValue>::Begin()
    .Field("fieldsList", &Vector4ComponentValue::fields)
    .Field("X", &Vector4ComponentValue::Get1Axis, &Vector4ComponentValue::Set1Axis)
    .Field("Y", &Vector4ComponentValue::Get2Axis, &Vector4ComponentValue::Set2Axis)
    .Field("Z", &Vector4ComponentValue::Get3Axis, &Vector4ComponentValue::Set3Axis)
    .Field("W", &Vector4ComponentValue::Get4Axis, &Vector4ComponentValue::Set4Axis)
    .Field("accuracy", &Vector4ComponentValue::GetAccuracy, nullptr)
    .Field("xRange", &Vector4ComponentValue::Get1AxisRange, nullptr)
    .Field("yRange", &Vector4ComponentValue::Get2AxisRange, nullptr)
    .Field("zRange", &Vector4ComponentValue::Get3AxisRange, nullptr)
    .Field("wRange", &Vector4ComponentValue::Get4AxisRange, nullptr)
    .End();
}

using RectComponentValue = kDComponentValue<Rect, MultiDoubleSpinBox, float32>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(RectComponentValue)
{
    ReflectionRegistrator<RectComponentValue>::Begin()
    .Field("fieldsList", &RectComponentValue::fields)
    .Field("X", &RectComponentValue::Get1Axis, &RectComponentValue::Set1Axis)
    .Field("Y", &RectComponentValue::Get2Axis, &RectComponentValue::Set2Axis)
    .Field("Width", &RectComponentValue::Get3Axis, &RectComponentValue::Set3Axis)
    .Field("Height", &RectComponentValue::Get4Axis, &RectComponentValue::Set4Axis)
    .Field("accuracy", &RectComponentValue::GetAccuracy, nullptr)
    .Field("xRange", &RectComponentValue::Get1AxisRange, nullptr)
    .Field("yRange", &RectComponentValue::Get2AxisRange, nullptr)
    .Field("widthRange", &RectComponentValue::Get3AxisRange, nullptr)
    .Field("heightRange", &RectComponentValue::Get4AxisRange, nullptr)
    .End();
}

using ColorComponentValue = kDComponentValue<Color, MultiDoubleSpinBox, float32>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(ColorComponentValue)
{
    ReflectionRegistrator<ColorComponentValue>::Begin()
    .Field("fieldsList", &ColorComponentValue::fields)
    .Field("R", &ColorComponentValue::Get1Axis, &ColorComponentValue::Set1Axis)
    .Field("G", &ColorComponentValue::Get2Axis, &ColorComponentValue::Set2Axis)
    .Field("B", &ColorComponentValue::Get3Axis, &ColorComponentValue::Set3Axis)
    .Field("A", &ColorComponentValue::Get4Axis, &ColorComponentValue::Set4Axis)
    .Field("value", &ColorComponentValue::GetFullValue, &ColorComponentValue::SetFillValue)
    .Field("accuracy", &ColorComponentValue::GetAccuracy, nullptr)
    .Field("rRange", &ColorComponentValue::Get1AxisRange, nullptr)
    .Field("gRange", &ColorComponentValue::Get2AxisRange, nullptr)
    .Field("bRange", &ColorComponentValue::Get3AxisRange, nullptr)
    .Field("aRange", &ColorComponentValue::Get4AxisRange, nullptr)
    .End();
}

using AABBox3ComponentValue = kDComponentValue<AABBox3, MultiDoubleSpinBox, float32>;
DAVA_VIRTUAL_TEMPLATE_SPECIALIZATION_REFLECTION_IMPL(AABBox3ComponentValue)
{
    ReflectionRegistrator<AABBox3ComponentValue>::Begin()
    .Field("fieldsList", &AABBox3ComponentValue::fields)
    .Field("Min X", &AABBox3ComponentValue::Get1Axis, &AABBox3ComponentValue::Set1Axis)
    .Field("Min Y", &AABBox3ComponentValue::Get2Axis, &AABBox3ComponentValue::Set2Axis)
    .Field("Min Z", &AABBox3ComponentValue::Get3Axis, &AABBox3ComponentValue::Set3Axis)
    .Field("Max X", &AABBox3ComponentValue::Get4Axis, &AABBox3ComponentValue::Set4Axis)
    .Field("Max Y", &AABBox3ComponentValue::Get5Axis, &AABBox3ComponentValue::Set5Axis)
    .Field("Max Z", &AABBox3ComponentValue::Get6Axis, &AABBox3ComponentValue::Set6Axis)
    .Field("accuracy", &AABBox3ComponentValue::GetAccuracy, nullptr)
    .Field("minXRange", &AABBox3ComponentValue::Get1AxisRange, nullptr)
    .Field("minYRange", &AABBox3ComponentValue::Get2AxisRange, nullptr)
    .Field("minZRange", &AABBox3ComponentValue::Get3AxisRange, nullptr)
    .Field("maxXRange", &AABBox3ComponentValue::Get4AxisRange, nullptr)
    .Field("maxYRange", &AABBox3ComponentValue::Get5AxisRange, nullptr)
    .Field("maxZRange", &AABBox3ComponentValue::Get6AxisRange, nullptr)
    .End();
}

#if __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wweak-template-vtables\"")
#endif

template class kDComponentValue<Vector2, MultiDoubleSpinBox, float32>;
template class kDComponentValue<Vector3, MultiDoubleSpinBox, float32>;
template class kDComponentValue<Vector4, MultiDoubleSpinBox, float32>;
template class kDComponentValue<Rect, MultiDoubleSpinBox, float32>;
template class kDComponentValue<Color, MultiDoubleSpinBox, float32>;
template class kDComponentValue<AABBox3, MultiDoubleSpinBox, float32>;

#if __clang__
_Pragma("clang diagnostic pop")
#endif

} // namespace TArc
} // namespace DAVA
