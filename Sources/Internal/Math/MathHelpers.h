#ifndef __DAVAENGINE_MATHHELPERS_H__
#define __DAVAENGINE_MATHHELPERS_H__

#include <cmath>
#include "Math/Math2D.h"
#include "Math/Vector.h"
#include "Math/Matrix4.h"
#include "Math/MathConstants.h"

namespace DAVA
{
/*
	Radians to degrees and back conversion functions and constants
	*/

static const float32 RAD_TO_DEG = 180.0f / 3.14159265358979323846f;
static const float32 DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

inline float32 RadToDeg(float32 f)
{
    return f * RAD_TO_DEG;
};
inline float32 DegToRad(float32 f)
{
    return f * DEG_TO_RAD;
};

inline void SinCosFast(float angleInRadians, float& sine, float& cosine)
{
    if (angleInRadians < 0.0f || angleInRadians >= PI_2)
    {
        angleInRadians -= std::floor(angleInRadians * (1.0f / PI_2)) * PI_2;
    }
    sine = PI - angleInRadians;
    if (Abs(sine) > PI_05)
    {
        sine = ((sine > 0.0f) ? PI : -PI) - sine;
        cosine = 1.0f;
    }
    else
    {
        cosine = -1.0f;
    }
    float a2 = sine * sine;
    sine *= ((0.00761f * a2 - 0.16605f) * a2 + 1.0f);
    cosine *= ((0.03705f * a2 - 0.49670f) * a2 + 1.0f);
}

inline float32 InvSqrtFast(float32 number) //only for IEEE 754 floating point format
{
    int32 i;
    float x2, y;
    const float32 threehalfs = 1.5f;

    x2 = number * 0.5f;
    y = number;
    i = *(reinterpret_cast<int32*>(&y));
    i = 0x5f3759df - (i >> 1);
    y = *(reinterpret_cast<float32*>(&i));
    y = y * (threehalfs - (x2 * y * y));

    return y;
}

/*
	Function to conver euler angles to normalized axial vectors
*/
void AnglesToVectors(const Vector3& _angles, Vector3& _vx, Vector3& _vy, Vector3& _vz);

template <typename T>
inline bool IsPowerOf2(T value)
{
    static_assert(std::is_integral<T>::value, "IsPowerOf2 works only with integral types");
    return value != 0 && ((value & (value - 1)) == 0);
}

inline int32 NextPowerOf2(int32 x)
{
    if (IsPowerOf2(x))
        return x;

    int32 ret = 1;

    while (ret < x)
    {
        ret *= 2;
    }

    return ret;
}

inline void EnsurePowerOf2(int32& x)
{
    x = NextPowerOf2(x);
}

template <typename T>
inline size_t HighestBitIndex(T value)
{
    static_assert(std::is_integral<T>::value, "HighestBitIndex works only with integral types");
    size_t index = 0;
    if (value != 0)
    {
        using UnsignedT = typename std::make_unsigned<T>::type;
        UnsignedT unsignedValue = static_cast<UnsignedT>(value);
        for (; unsignedValue != 0; ++index)
        {
            unsignedValue >>= 1;
        }
        index -= 1;
    }
    return index;
}

template <typename T>
inline T Sign(T val)
{
    if (val == 0)
    {
        return 0;
    }
    return T(val > 0 ? 1 : -1);
}

/*
	Function to get intersection point of 
	vector (start + dir) 
	with plane (plane normal + plane point)
	*/
DAVA_DEPRECATED(inline bool GetIntersectionVectorWithPlane(const Vector3& start, const Vector3& dir, const Vector3& planeN, const Vector3& planePoint, Vector3& result));
inline bool GetIntersectionVectorWithPlane(const Vector3& start, const Vector3& dir, const Vector3& planeN, const Vector3& planePoint, Vector3& result)
{
    Vector3 intersection;
    float32 cosang, dist, lamda;

    float32 d = planeN.DotProduct(planePoint);

    cosang = dir.DotProduct(planeN);
    if (cosang > -EPSILON && cosang < EPSILON)
    {
        //this is parallels
        return false;
    }

    dist = start.DotProduct(planeN);

    lamda = (d - dist) / cosang;
    if (lamda < 0)
    {
        //this not intersect
        return false;
    }
    result = start + dir * lamda;
    return true;
}

/*
	================
	SquareRootFloat
	================
	*/
inline float32 SquareRootFloat(float32 number)
{
    int32 i;
    float32 x, y;
    const float32 f = 1.5f;

    x = number * 0.5f;
    y = number;
    i = *(reinterpret_cast<int32*>(&y));
    i = 0x5f3759df - (i >> 1);
    y = *(reinterpret_cast<float32*>(&i));
    y = y * (f - (x * y * y));
    y = y * (f - (x * y * y));
    return number * y;
}

inline Vector3& TransformPerserveLength(Vector3& vec, const Matrix3& mat)
{
    float32 oldLength = vec.SquareLength();
    vec = vec * mat;
    float newLength = vec.SquareLength();
    if (newLength > EPSILON)
        vec *= std::sqrt(oldLength / newLength);
    return vec;
}

inline Vector3& TransformPerserveLength(Vector3& vec, const Matrix4& mat)
{
    float32 oldLength = vec.SquareLength();
    vec = vec * mat;
    float newLength = vec.SquareLength();
    if (newLength > EPSILON)
        vec *= std::sqrt(oldLength / newLength);
    return vec;
}

inline float32 Round(float32 value)
{
    return (value > 0.0f) ? std::floor(value + 0.5f) : std::ceil(value - 0.5f);
}

inline Vector3 Polar(DAVA::float32 angle, DAVA::float32 distance)
{
    return DAVA::Vector3(std::cos(angle) * distance, std::sin(angle) * distance, 0.0f);
};

} // end of namespace DAVA

#endif
