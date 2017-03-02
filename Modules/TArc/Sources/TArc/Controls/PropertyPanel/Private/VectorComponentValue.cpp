#include "TArc/Controls/PropertyPanel/Private/VectorComponentValue.h"
#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace TArc
{
namespace VectorComponentValueDetail
{
template <typename T>
struct VectorTraits
{
    template <int32 Index>
    static float32 Get(const T& v)
    {
        return 0.0;
    }

    template <int32 Index>
    static void Set(T& vec, float32 v)
    {
        return;
    }

    static void InitFieldsList(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
    {
        DVASSERT(false);
    }

    static void InitRanges(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 4>& ranges)
    {
        //DVASSERT(false);
    }
};

template <>
struct VectorTraits<Vector2>
{
    template <int32 Index>
    static float32 Get(const Vector2& v)
    {
        DVASSERT(Index < 2);
        return v.data[Index];
    }

    template <int32 Index>
    static void Set(Vector2& vec, float32 v)
    {
        DVASSERT(Index < 2);
        vec.data[Index] = v;
    }

    static void InitFieldsList(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
    {
        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "X";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "xRange";
            fields.push_back(descr);
        }

        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "Y";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "yRange";
            fields.push_back(descr);
        }
    }

    static void InitRanges(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 4>& ranges)
    {
        //DVASSERT(false);
    }
};

template <>
struct VectorTraits<Vector3>
{
    template <int32 Index>
    static float32 Get(const Vector3& v)
    {
        DVASSERT(Index < 3);
        return v.data[Index];
    }

    template <int32 Index>
    static void Set(Vector3& vec, float32 v)
    {
        DVASSERT(Index < 3);
        vec.data[Index] = v;
    }

    static void InitFieldsList(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
    {
        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "X";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "xRange";
            fields.push_back(descr);
        }

        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "Y";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "yRange";
            fields.push_back(descr);
        }

        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "Z";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "zRange";
            fields.push_back(descr);
        }
    }

    static void InitRanges(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 4>& ranges)
    {
        //DVASSERT(false);
    }
};

template <>
struct VectorTraits<Vector4>
{
    template <int32 Index>
    static float32 Get(const Vector4& v)
    {
        DVASSERT(Index < 4);
        return v.data[Index];
    }

    template <int32 Index>
    static void Set(Vector4& vec, float32 v)
    {
        DVASSERT(Index < 4);
        vec.data[Index] = v;
    }

    static void InitFieldsList(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
    {
        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "X";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "xRange";
            fields.push_back(descr);
        }

        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "Y";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "yRange";
            fields.push_back(descr);
        }

        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "Z";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "zRange";
            fields.push_back(descr);
        }

        {
            MultiDoubleSpinBox::FieldDescriptor descr;
            descr.valueRole = "W";
            descr.accuracyRole = "accuracy";
            descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
            descr.rangeRole = "wRange";
            fields.push_back(descr);
        }
    }

    static void InitRanges(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 4>& ranges)
    {
        //DVASSERT(false);
    }
};

template <typename T, int32 Index>
static Any GetAxisValue(const Vector<std::shared_ptr<PropertyNode>>& nodes)
{
    float32 v = VectorTraits<T>::Get<Index>(nodes.front()->cachedValue.Cast<T>());
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (v != VectorTraits<T>::Get<Index>(node->cachedValue.Cast<T>()))
        {
            return Any();
        }
    }

    return v;
}

template <typename T, int32 Index>
static void SetAxisValue(const Vector<std::shared_ptr<PropertyNode>>& nodes, float32 v, ModifyExtension::MultiCommandInterface& cmdInterface)
{
    for (const std::shared_ptr<PropertyNode>& node : nodes)
    {
        T vec = node->cachedValue.Cast<T>();
        if (VectorTraits<T>::Get<Index>(vec) != v)
        {
            VectorTraits<T>::Set<Index>(vec, v);
            cmdInterface.ModifyPropertyValue(node, vec);
        }
    }
}
}

template <typename T>
Any VectorComponentValue<T>::GetMultipleValue() const
{
    return Any();
}

template <typename T>
bool VectorComponentValue<T>::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    return false;
}

template <typename T>
ControlProxy* VectorComponentValue<T>::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const
{
    using namespace VectorComponentValueDetail;
    VectorComponentValue<T>* nonConstThis = const_cast<VectorComponentValue<T>*>(this);
    VectorTraits<T>::InitFieldsList(nonConstThis->fields);
    VectorTraits<T>::InitRanges(nodes, nonConstThis->ranges);

    ControlDescriptorBuilder<MultiDoubleSpinBox::Fields> descr;
    descr[MultiDoubleSpinBox::Fields::FieldsList] = "fieldsList";
    return new MultiDoubleSpinBox(descr, wrappersProcessor, model, parent);
}

template <typename T>
Any VectorComponentValue<T>::GetX() const
{
    using namespace VectorComponentValueDetail;
    return GetAxisValue<T, 0>(nodes);
}

template <typename T>
Any VectorComponentValue<T>::GetY() const
{
    using namespace VectorComponentValueDetail;
    return GetAxisValue<T, 1>(nodes);
}

template <typename T>
Any VectorComponentValue<T>::GetZ() const
{
    using namespace VectorComponentValueDetail;
    return GetAxisValue<T, 2>(nodes);
}

template <typename T>
Any VectorComponentValue<T>::GetW() const
{
    using namespace VectorComponentValueDetail;
    return GetAxisValue<T, 3>(nodes);
}

template <typename T>
void VectorComponentValue<T>::SetX(const Any& x)
{
    using namespace VectorComponentValueDetail;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetAxisValue<T, 0>(nodes, x.Cast<float32>(), cmdInterface);
}

template <typename T>
void VectorComponentValue<T>::SetY(const Any& y)
{
    using namespace VectorComponentValueDetail;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetAxisValue<T, 1>(nodes, y.Cast<float32>(), cmdInterface);
}

template <typename T>
void VectorComponentValue<T>::SetZ(const Any& z)
{
    using namespace VectorComponentValueDetail;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetAxisValue<T, 2>(nodes, z.Cast<float32>(), cmdInterface);
}

template <typename T>
void VectorComponentValue<T>::SetW(const Any& w)
{
    using namespace VectorComponentValueDetail;
    ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(static_cast<uint32>(nodes.size()));
    SetAxisValue<T, 3>(nodes, w.Cast<float32>(), cmdInterface);
}

template <typename T>
int32 VectorComponentValue<T>::GetAccuracy() const
{
    if (!nodes.empty())
    {
        const M::FloatNumberAccuracy* accuracy = nodes.front()->field.ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracy != nullptr)
        {
            return accuracy->accuracy;
        }
    }

    return 6;
}

template <typename T>
const M::Range* VectorComponentValue<T>::GetXRange() const
{
    return ranges[0].get();
}

template <typename T>
const M::Range* VectorComponentValue<T>::GetYRange() const
{
    return ranges[1].get();
}

template <typename T>
const M::Range* VectorComponentValue<T>::GetZRange() const
{
    return ranges[2].get();
}

template <typename T>
const M::Range* VectorComponentValue<T>::GetWRange() const
{
    return ranges[3].get();
}

template <typename T>
Vector<MultiDoubleSpinBox::FieldDescriptor> VectorComponentValue<T>::GetFieldsList() const
{
    return fields;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VectorComponentValue<Vector2>)
{
    ReflectionRegistrator<VectorComponentValue<Vector2>>::Begin()
    .Field("fieldsList", &VectorComponentValue<Vector2>::fields)
    .Field("X", &VectorComponentValue<Vector2>::GetX, &VectorComponentValue<Vector2>::SetX)
    .Field("Y", &VectorComponentValue<Vector2>::GetY, &VectorComponentValue<Vector2>::SetY)
    .Field("accuracy", &VectorComponentValue<Vector2>::GetAccuracy, nullptr)
    .Field("xRange", &VectorComponentValue<Vector2>::GetXRange, nullptr)
    .Field("yRange", &VectorComponentValue<Vector2>::GetYRange, nullptr)
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(VectorComponentValue<Vector3>)
{
    ReflectionRegistrator<VectorComponentValue<Vector3>>::Begin()
    .Field("fieldsList", &VectorComponentValue<Vector3>::fields)
    .Field("X", &VectorComponentValue<Vector3>::GetX, &VectorComponentValue<Vector3>::SetX)
    .Field("Y", &VectorComponentValue<Vector3>::GetY, &VectorComponentValue<Vector3>::SetY)
    .Field("Z", &VectorComponentValue<Vector3>::GetZ, &VectorComponentValue<Vector3>::SetZ)
    .Field("accuracy", &VectorComponentValue<Vector3>::GetAccuracy, nullptr)
    .Field("xRange", &VectorComponentValue<Vector3>::GetXRange, nullptr)
    .Field("yRange", &VectorComponentValue<Vector3>::GetYRange, nullptr)
    .Field("zRange", &VectorComponentValue<Vector3>::GetZRange, nullptr)
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(VectorComponentValue<Vector4>)
{
    ReflectionRegistrator<VectorComponentValue<Vector4>>::Begin()
    .Field("fieldsList", &VectorComponentValue<Vector4>::fields)
    .Field("X", &VectorComponentValue<Vector4>::GetX, &VectorComponentValue<Vector4>::SetX)
    .Field("Y", &VectorComponentValue<Vector4>::GetY, &VectorComponentValue<Vector4>::SetY)
    .Field("Z", &VectorComponentValue<Vector4>::GetZ, &VectorComponentValue<Vector4>::SetZ)
    .Field("W", &VectorComponentValue<Vector4>::GetW, &VectorComponentValue<Vector4>::SetW)
    .Field("accuracy", &VectorComponentValue<Vector4>::GetAccuracy, nullptr)
    .Field("xRange", &VectorComponentValue<Vector4>::GetXRange, nullptr)
    .Field("yRange", &VectorComponentValue<Vector4>::GetYRange, nullptr)
    .Field("zRange", &VectorComponentValue<Vector4>::GetZRange, nullptr)
    .Field("wRange", &VectorComponentValue<Vector4>::GetWRange, nullptr)
    .End();
}

#if __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wweak-template-vtables\"")
#endif

template class VectorComponentValue<Vector2>;
template class VectorComponentValue<Vector3>;
template class VectorComponentValue<Vector4>;

#if __clang__
_Pragma("clang diagnostic pop")
#endif

} // namespace TArc
} // namespace DAVA
