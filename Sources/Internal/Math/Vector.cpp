#include "Vector.h"
#include "Matrix3.h"

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

template <>
bool AnyCompare<Vector2>::IsEqual(const Any& v1, const Any& v2)
{
    const Vector2& vec1 = v1.Get<Vector2>();
    const Vector2& vec2 = v2.Get<Vector2>();
    return vec1 == vec2;
};

template <>
bool AnyCompare<Vector3>::IsEqual(const Any& v1, const Any& v2)
{
    const Vector3& vec1 = v1.Get<Vector3>();
    const Vector3& vec2 = v2.Get<Vector3>();
    return vec1 == vec2;
};

template <>
bool AnyCompare<Vector4>::IsEqual(const Any& v1, const Any& v2)
{
    const Vector4& vec1 = v1.Get<Vector4>();
    const Vector4& vec2 = v2.Get<Vector4>();
    return vec1 == vec2;
};
}
