#include "Vector.h"
#include "Matrix3.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
const Vector2 Vector2::Zero(0.0f, 0.0f);
const Vector2 Vector2::UnitX(1.0f, 0.0f);
const Vector2 Vector2::UnitY(0.0f, 1.0f);

const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);

const Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);

Vector2 Rotate(const Vector2& in, float32 angleRad)
{
    DAVA::Matrix3 rotateMatrix;
    rotateMatrix.BuildRotation(angleRad);
    return in * rotateMatrix;
}

DAVA_REFLECTION_IMPL(Vector2)
{
    ReflectionRegistrator<Vector2>::Begin()
    .Field("X", &Vector2::x)[M::SubProperty()]
    .Field("Y", &Vector2::y)[M::SubProperty()]
    .End();
}

DAVA_REFLECTION_IMPL(Vector3)
{
    ReflectionRegistrator<Vector3>::Begin()
    .Field("X", &Vector3::x)[M::SubProperty()]
    .Field("Y", &Vector3::y)[M::SubProperty()]
    .Field("Z", &Vector3::z)[M::SubProperty()]
    .End();
}

DAVA_REFLECTION_IMPL(Vector4)
{
    ReflectionRegistrator<Vector4>::Begin()
    .Field("X", &Vector4::x)[M::SubProperty()]
    .Field("Y", &Vector4::y)[M::SubProperty()]
    .Field("Z", &Vector4::z)[M::SubProperty()]
    .Field("W", &Vector4::w)[M::SubProperty()]
    .End();
}

template <>
bool AnyCompare<Vector2>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<Vector2>() == v2.Get<Vector2>();
}

template <>
bool AnyCompare<Vector3>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<Vector3>() == v2.Get<Vector3>();
}

template <>
bool AnyCompare<Vector4>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<Vector4>() == v2.Get<Vector4>();
}
}
