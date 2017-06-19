#pragma once

#include <Math/Matrix4.h>
#include <Base/BaseTypes.h>

#include <physx/PxPhysicsAPI.h>

namespace DAVA
{
namespace PhysicsMath
{
inline physx::PxMat44 Matrix4ToPxMat44(const Matrix4& in)
{
    physx::PxMat44 out;

    out.column0.x = in.data[0];
    out.column0.y = in.data[1];
    out.column0.z = in.data[2];
    out.column0.w = in.data[3];

    out.column1.x = in.data[4];
    out.column1.y = in.data[5];
    out.column1.z = in.data[6];
    out.column1.w = in.data[7];

    out.column2.x = in.data[8];
    out.column2.y = in.data[9];
    out.column2.z = in.data[10];
    out.column2.w = in.data[11];

    out.column3.x = in.data[12];
    out.column3.y = in.data[13];
    out.column3.z = in.data[14];
    out.column3.w = in.data[15];

    return out;
}

inline Matrix4 PxMat44ToMatrix4(const physx::PxMat44& in)
{
    return Matrix4(in.column0.x, in.column0.y, in.column0.z, in.column0.w,
                   in.column1.x, in.column1.y, in.column1.z, in.column1.w,
                   in.column2.x, in.column2.y, in.column2.z, in.column2.w,
                   in.column3.x, in.column3.y, in.column3.z, in.column3.w);
}

inline physx::PxVec3 Vector3ToPxVec3(const Vector3& in)
{
    return physx::PxVec3(in.x, in.y, in.z);
}

} // namespace PhysicsMath
} // namespace DAVA
