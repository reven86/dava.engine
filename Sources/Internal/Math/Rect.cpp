#include "Math/Rect.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_REFLECTION_IMPL(Rect)
{
    ReflectionRegistrator<Rect>::Begin()
    .Field("X", &Rect::x)[M::SubProperty()]
    .Field("Y", &Rect::y)[M::SubProperty()]
    .Field("Width", &Rect::dx)[M::SubProperty()]
    .Field("Height", &Rect::dy)[M::SubProperty()]
    .End();
}

template <>
bool AnyCompare<Rect>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<Rect>() == v2.Get<Rect>();
}
} // namespace DAVA