#include "Math/AABBox3.h"

namespace DAVA
{
//! \brief check if bounding box intersect ray
bool AABBox3::IsIntersectsWithRay(Ray3& r, float32& tmin, float32& tmax, float32 t0, float32 t1)
{
    float32 tymin, tymax, tzmin, tzmax;

    float32 divx = 1.0f / r.direction.x;
    if (divx >= 0)
    {
        tmin = (min.x - r.origin.x) * divx;
        tmax = (max.x - r.origin.x) * divx;
    }
    else
    {
        tmin = (max.x - r.origin.x) * divx;
        tmax = (min.x - r.origin.x) * divx;
    }

    float32 divy = 1.0f / r.direction.y;
    if (divy >= 0)
    {
        tymin = (min.y - r.origin.y) * divy;
        tymax = (max.y - r.origin.y) * divy;
    }
    else
    {
        tymin = (max.y - r.origin.y) * divy;
        tymax = (min.y - r.origin.y) * divy;
    }
    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    float32 divz = 1.0f / r.direction.z;
    if (divz >= 0)
    {
        tzmin = (min.z - r.origin.z) * divz;
        tzmax = (max.z - r.origin.z) * divz;
    }
    else
    {
        tzmin = (max.z - r.origin.z) * divz;
        tzmax = (min.z - r.origin.z) * divz;
    }

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;
    return ((tmin < t1) && (tmax > t0));
}

void AABBox3::GetTransformedBox(const Matrix4& transform, AABBox3& result) const
{
    if (IsEmpty())
    {
        result.Empty();
        return;
    }

    result.min.x = transform.data[12];
    result.min.y = transform.data[13];
    result.min.z = transform.data[14];

    result.max.x = transform.data[12];
    result.max.y = transform.data[13];
    result.max.z = transform.data[14];

    for (int32 i = 0; i < 3; ++i)
    {
        for (int32 j = 0; j < 3; ++j)
        {
            float32 a = transform._data[j][i] * min.data[j];
            float32 b = transform._data[j][i] * max.data[j];

            if (a < b)
            {
                result.min.data[i] += a;
                result.max.data[i] += b;
            }
            else
            {
                result.min.data[i] += b;
                result.max.data[i] += a;
            }
        };
    }
}

void AABBox3::GetCorners(Vector3* cornersArray) const
{
    cornersArray[0] = min;
    cornersArray[1] = max;
    cornersArray[2].Set(min.x, min.y, max.z);
    cornersArray[3].Set(min.x, max.y, min.z);
    cornersArray[4].Set(max.x, min.y, min.z);
    cornersArray[5].Set(max.x, max.y, min.z);
    cornersArray[6].Set(max.x, min.y, max.z);
    cornersArray[7].Set(min.x, max.y, max.z);
}

float32 AABBox3::GetBoundingSphereRadius() const
{
    Vector3 maxToCenter = max - GetCenter();
    Vector3 minToCenter = min - GetCenter();
    float32 maxCoord = Max(maxToCenter.x, Max(maxToCenter.y, maxToCenter.z));
    float32 minCoord = Max(Abs(minToCenter.x), Max(Abs(minToCenter.y), Abs(minToCenter.z)));
    return Max(maxCoord, minCoord);
}

AABBox3 AABBox3::GetMaxRotationExtentBox(const Vector3& rotationCenter) const
{
    float32 rotationRadius = (GetCenter() - rotationCenter).Length();
    return AABBox3(rotationCenter, 2.0f * (rotationRadius + GetBoundingSphereRadius()));
}

template <>
bool AnyCompare<AABBox3>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<AABBox3>() == v2.Get<AABBox3>();
}
};
