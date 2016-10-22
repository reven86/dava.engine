#pragma once

#include "Neon/NeonMath.h"
#include "Math/Matrix3.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
//
//
//	we use vectors as rows  (row-major matrixes)
//	DirectX, COLLADA matrixes are row-major
//	OpenGL matrixes are column-major
//
//	v = [ x y z w ]
//	m = [ _00 _01 _02 _03 ]
//		[ _10 _11 _12 _13 ]
//		[ _20 _21 _22 _23 ]
//		[ _30 _31 _32 _33 ]
//

/**	
	\ingroup math
	\brief Class to work with 4 x 4 matrices.
 */

class Quaternion;

struct Matrix4
{
    union
    {
        float32 data[16];
        float32 _data[4][4];
        struct
        {
            float32 _00, _01, _02, _03;
            float32 _10, _11, _12, _13;
            float32 _20, _21, _22, _23;
            float32 _30, _31, _32, _33;
        };
    };
    inline Matrix4();

    inline Matrix4(float32 _D00, float32 _D01, float32 _D02, float32 _D03,
                   float32 _D10, float32 _D11, float32 _D12, float32 _D13,
                   float32 _D20, float32 _D21, float32 _D22, float32 _D23,
                   float32 _D30, float32 _D31, float32 _D32, float32 _D33);

    inline explicit Matrix4(const Matrix3& m);
    inline Matrix4(const Matrix4& m);

    void Dump();

    inline Matrix4& operator=(const Matrix4& m);

    inline void Identity();
    inline void Zero();

    inline void BuildProjectionFovLH(float32 _fovY, float32 _aspect, float32 _zn, float32 _zf);
    inline void BuildOrthoLH(float32 _l, float32 _r, float32 _t, float32 _b, float32 _zn, float32 _zf);

    /*
        Convenience functions to simplify movement to own matrices. 
        Temporary solution. 
     */
    void glOrtho(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far, bool zeroBaseClipRange);
    void glFrustum(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far, bool zeroBaseClipRange);

    void glRotate(float32 angle, float32 x, float32 y, float32 z);
    void glTranslate(float32 x, float32 y, float32 z);
    void glScale(float32 x, float32 y, float32 z);

    inline void BuildLookAtMatrixLH(const Vector3& _position,
                                    const Vector3& _target,
                                    const Vector3& _up);
    inline void BuildLookAtMatrixRH(const Vector3& _position,
                                    const Vector3& _target,
                                    const Vector3& _up);

    inline void Transpose();

    inline bool Inverse();
    inline bool GetInverse(Matrix4& out) const;
    inline bool Decomposition(Vector3& position, Vector3& scale, Vector3& orientation) const;
    void Decomposition(Vector3& position, Vector3& scale, Quaternion& orientation) const;

    //!
    inline static Matrix4 MakeTranslation(const Vector3& translationVector);
    inline static Matrix4 MakeRotation(const Vector3& axis, float32 angleInRadians);
    inline static Matrix4 MakeScale(const Vector3& scaleVector);

    //! create translation matrix
    inline void CreateTranslation(const Vector3& vector);

    //! create rotation matrix
    inline void CreateRotation(const Vector3& axis, float32 angleInRadians);

    //! create scale matrix
    inline void CreateScale(const Vector3& vector);

    inline Vector3 GetTranslationVector() const;
    inline void SetTranslationVector(const Vector3& vector);

    inline Vector3 GetScaleVector() const;

    //! Simple operator for directly accessing every element of the matrix.
    float32& operator()(int32 row, int32 col)
    {
        return _data[col][row];
    }
    //! Simple operator for directly accessing every element of the matrix.
    const float32& operator()(int32 row, int32 col) const
    {
        return _data[col][row];
    }

    //! matrix mul vector
    //inline Vector3 operator * (const Vector3 & ) const;
    //inline Vector4 operator * (const Vector4 & ) const;

    //! matrix multiplication
    inline Matrix4 operator*(const Matrix4& m) const;
    inline const Matrix4& operator*=(const Matrix4& m);
    inline operator Matrix3() const;

    //! Comparison operators
    inline bool operator==(const Matrix4& _m) const;
    inline bool operator!=(const Matrix4& _m) const;

    static Matrix4 IDENTITY;
    //static uint32 matrixMultiplicationCounter;
};

inline Vector3 operator*(const Vector3& _v, const Matrix4& _m);
// Function to perform matrix multiplication without creation of Matrix3
inline Vector3 MultiplyVectorMat3x3(const Vector3& _v, const Matrix4& _m);
inline Vector2 MultiplyVectorMat2x2(const Vector2& _v, const Matrix4& _m);

// Implementation of matrix4

inline Matrix4::Matrix4()
{
    // 	_00 = 0; _01 = 0; _02 = 0; _03 = 0;
    // 	_10 = 0; _11 = 0; _12 = 0; _13 = 0;
    // 	_20 = 0; _21 = 0; _22 = 0; _23 = 0;
    // 	_30 = 0; _31 = 0; _32 = 0; _33 = 0;
    Identity();
}

inline Matrix4::Matrix4(float32 _D00, float32 _D01, float32 _D02, float32 _D03,
                        float32 _D10, float32 _D11, float32 _D12, float32 _D13,
                        float32 _D20, float32 _D21, float32 _D22, float32 _D23,
                        float32 _D30, float32 _D31, float32 _D32, float32 _D33)
{
    _00 = _D00;
    _01 = _D01;
    _02 = _D02;
    _03 = _D03;
    _10 = _D10;
    _11 = _D11;
    _12 = _D12;
    _13 = _D13;
    _20 = _D20;
    _21 = _D21;
    _22 = _D22;
    _23 = _D23;
    _30 = _D30;
    _31 = _D31;
    _32 = _D32;
    _33 = _D33;
}

inline Matrix4::Matrix4(const Matrix4& m)
{
    // 	_00 = m._00; _01 = m._01; _02 = m._02; _03 = m._03;
    // 	_10 = m._10; _11 = m._11; _12 = m._12; _13 = m._13;
    // 	_20 = m._20; _21 = m._21; _22 = m._22; _23 = m._23;
    // 	_30 = m._30; _31 = m._31; _32 = m._32; _33 = m._33;
    *this = m;
}

inline Matrix4::Matrix4(const Matrix3& m)
{
    _00 = m._00;
    _01 = m._01;
    _02 = m._02;
    _03 = 0.0f;
    _10 = m._10;
    _11 = m._11;
    _12 = m._12;
    _13 = 0.0f;
    _20 = m._20;
    _21 = m._21;
    _22 = m._22;
    _23 = 0.0f;
    _30 = 0.0f;
    _31 = 0.0f;
    _32 = 0.0f;
    _33 = 1.0f;
}

inline Matrix4& Matrix4::operator=(const Matrix4& m)
{
    // 	_00 = m._00; _01 = m._01; _02 = m._02; _03 = m._03;
    // 	_10 = m._10; _11 = m._11; _12 = m._12; _13 = m._13;
    // 	_20 = m._20; _21 = m._21; _22 = m._22; _23 = m._23;
    // 	_30 = m._30; _31 = m._31; _32 = m._32; _33 = m._33;
    Memcpy(data, m.data, 16 * sizeof(float32));
    return *this;
}

inline Matrix4::operator Matrix3() const
{
    Matrix3 m;
    m._00 = _00;
    m._01 = _01;
    m._02 = _02;
    m._10 = _10;
    m._11 = _11;
    m._12 = _12;
    m._20 = _20;
    m._21 = _21;
    m._22 = _22;
    return m;
}

inline void Matrix4::Identity()
{
    data[0] = 1.0f;
    data[1] = 0.0f;
    data[2] = 0.0f;
    data[3] = 0.0f;
    data[4] = 0.0f;
    data[5] = 1.0f;
    data[6] = 0.0f;
    data[7] = 0.0f;
    data[8] = 0.0f;
    data[9] = 0.0f;
    data[10] = 1.0f;
    data[11] = 0.0f;
    data[12] = 0.0f;
    data[13] = 0.0f;
    data[14] = 0.0f;
    data[15] = 1.0f;
}

inline void Matrix4::Zero()
{
    // 	data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0;
    // 	data[4] = 0; data[5] = 0; data[6] = 0; data[7] = 0;
    // 	data[8] = 0; data[9] = 0; data[10] = 0; data[11] = 0;
    // 	data[12] = 0; data[13] = 0; data[14] = 0; data[15] = 0;
    Memset(data, 0, 16 * sizeof(float32));
}

inline void Matrix4::BuildProjectionFovLH(float32 _fovY, float32 _aspect, float32 _zn, float32 _zf)
{
    // DX9 formula
    Zero();

    float32 sinF2 = std::sin(_fovY / 2.0f);
    float32 cosF2 = std::cos(_fovY / 2.0f);

    float h = cosF2 / sinF2;
    float w = h / _aspect;

    _data[0][0] = w;
    _data[1][1] = h;
    _data[2][2] = _zf / (_zf - _zn);
    _data[3][2] = (-_zn * _zf) / (_zf - _zn);
    _data[2][3] = 1.0f;
}

inline void Matrix4::BuildOrthoLH(float32 _l, float32 _r, float32 _b, float32 _t, float32 _zn, float32 _zf)
{
    // DX9 formula
    Zero();

    _data[0][0] = 2.0f / (_r - _l);
    _data[1][1] = 2.0f / (_t - _b);
    _data[2][2] = 1 / (_zf - _zn);

    _data[3][2] = (_zn) / (_zn - _zf);
    _data[3][0] = (_l + _r) / (_l - _r);
    _data[3][1] = (_b + _t) / (_b - _t);
    _data[3][3] = 1.0f;
}

inline void Matrix4::BuildLookAtMatrixLH(
const Vector3& _position,
const Vector3& _target,
const Vector3& _up)
{
    //
    //	float32 m[16];
    //    float32 x[3], y[3], z[3];
    //    float32 mag;
    //
    //    /* Make rotation matrix */
    //
    //    /* Z vector */
    //    z[0] = eye.x - center.x;
    //    z[1] = eye.y - center.y;
    //    z[2] = eye.z - center.z;
    //    mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    //    if (mag) {          /* mpichler, 19950515 */
    //        z[0] /= mag;
    //        z[1] /= mag;
    //        z[2] /= mag;
    //    }
    //
    //    /* Y vector */
    //    y[0] = up.x;
    //    y[1] = up.y;
    //    y[2] = up.z;
    //
    //    /* X vector = Y cross Z */
    //    x[0] = y[1] * z[2] - y[2] * z[1];
    //    x[1] = -y[0] * z[2] + y[2] * z[0];
    //    x[2] = y[0] * z[1] - y[1] * z[0];
    //
    //    /* Recompute Y = Z cross X */
    //    y[0] = z[1] * x[2] - z[2] * x[1];
    //    y[1] = -z[0] * x[2] + z[2] * x[0];
    //    y[2] = z[0] * x[1] - z[1] * x[0];
    //
    //    /* mpichler, 19950515 */
    //    /* cross product gives area of parallelogram, which is < 1.0 for
    //     * non-perpendicular unit-length vectors; so normalize x, y here
    //     */
    //
    //    mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    //    if (mag) {
    //        x[0] /= mag;
    //        x[1] /= mag;
    //        x[2] /= mag;
    //    }
    //
    //    mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    //    if (mag) {
    //        y[0] /= mag;
    //        y[1] /= mag;
    //        y[2] /= mag;
    //    }
    //
    //#define M(row,col)  m[col*4+row]
    //    M(0, 0) = x[0];
    //    M(0, 1) = x[1];
    //    M(0, 2) = x[2];
    //    M(0, 3) = 0.0;
    //    M(1, 0) = y[0];
    //    M(1, 1) = y[1];
    //    M(1, 2) = y[2];
    //    M(1, 3) = 0.0;
    //    M(2, 0) = z[0];
    //    M(2, 1) = z[1];
    //    M(2, 2) = z[2];
    //    M(2, 3) = 0.0;
    //    M(3, 0) = 0.0;
    //    M(3, 1) = 0.0;
    //    M(3, 2) = 0.0;
    //    M(3, 3) = 1.0;
    //#undef M
    //
    //	for (int32 k = 0; k < 16; ++k)
    //		data[k] = m[k];
    //
    //	Matrix4 translation;
    //	translation.CreateTranslation(- eye);
    //	*this = *this * translation;

    Vector3 left, forward, up;

    forward = _target - _position;
    forward.Normalize();

    left = CrossProduct(_up, forward);
    left.Normalize();

    up = CrossProduct(forward, left);
    up.Normalize();

    Identity();

    Vector3& vx = left;
    Vector3& vy = up;
    Vector3& vz = forward;

    _data[0][0] = vx.x;
    _data[1][0] = vx.y;
    _data[2][0] = vx.z;
    _data[3][0] = -_position.DotProduct(vx);

    _data[0][1] = vy.x;
    _data[1][1] = vy.y;
    _data[2][1] = vy.z;
    _data[3][1] = -_position.DotProduct(vy);

    _data[0][2] = vz.x;
    _data[1][2] = vz.y;
    _data[2][2] = vz.z;
    _data[3][2] = -_position.DotProduct(vz);

    //*this = *this * translation;
}

inline void Matrix4::BuildLookAtMatrixRH(
const Vector3& _position,
const Vector3& _target,
const Vector3& _up)
{
    Vector3 left, forward, up;

    forward = _position - _target;
    forward.Normalize();

    left = CrossProduct(_up, forward);
    left.Normalize();

    up = CrossProduct(forward, left);
    up.Normalize();

    Identity();

    Vector3& vx = left;
    Vector3& vy = up;
    Vector3& vz = forward;

    _data[0][0] = vx.x;
    _data[1][0] = vx.y;
    _data[2][0] = vx.z;
    _data[3][0] = -_position.DotProduct(vx);

    _data[0][1] = vy.x;
    _data[1][1] = vy.y;
    _data[2][1] = vy.z;
    _data[3][1] = -_position.DotProduct(vy);

    _data[0][2] = vz.x;
    _data[1][2] = vz.y;
    _data[2][2] = vz.z;
    _data[3][2] = -_position.DotProduct(vz);
}

inline Vector3 operator*(const Vector3& _v, const Matrix4& _m)
{
    Vector3 res;

    res.x = _v.x * _m._00 + _v.y * _m._10 + _v.z * _m._20 + _m._30;
    res.y = _v.x * _m._01 + _v.y * _m._11 + _v.z * _m._21 + _m._31;
    res.z = _v.x * _m._02 + _v.y * _m._12 + _v.z * _m._22 + _m._32;

    return res;
}

inline Vector4 operator*(const Vector4& _v, const Matrix4& _m)
{
    Vector4 res;

    res.x = _v.x * _m._00 + _v.y * _m._10 + _v.z * _m._20 + _v.w * _m._30;
    res.y = _v.x * _m._01 + _v.y * _m._11 + _v.z * _m._21 + _v.w * _m._31;
    res.z = _v.x * _m._02 + _v.y * _m._12 + _v.z * _m._22 + _v.w * _m._32;
    res.w = _v.x * _m._03 + _v.y * _m._13 + _v.z * _m._23 + _v.w * _m._33;

    return res;
}

inline Vector3 MultiplyVectorMat3x3(const Vector3& _v, const Matrix4& _m)
{
    Vector3 res;
    res.x = _v.x * _m._00 + _v.y * _m._10 + _v.z * _m._20;
    res.y = _v.x * _m._01 + _v.y * _m._11 + _v.z * _m._21;
    res.z = _v.x * _m._02 + _v.y * _m._12 + _v.z * _m._22;
    return res;
}

inline Vector2 MultiplyVectorMat2x2(const Vector2& _v, const Matrix4& _m)
{
    Vector2 res;
    res.x = _v.x * _m._00 + _v.y * _m._10;
    res.y = _v.x * _m._01 + _v.y * _m._11;
    return res;
}

inline void Matrix4::Transpose()
{
    Matrix4 t;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            t._data[i][j] = _data[j][i];
    *this = t;
}

inline bool Matrix4::GetInverse(Matrix4& out) const
{
    /// Calculates the inverse of this Matrix
    /// The inverse is calculated using Cramers rule.
    /// If no inverse exists then 'false' is returned.
    const Matrix4& m = *this;

    float32 d = (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3)) - (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3))
    + (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3)) + (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3))
    - (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3)) + (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3));

    if (d == 0.f)
        return false;

    d = 1.f / d;

    out(0, 0) = d * (m(1, 1) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3)) + m(2, 1) * (m(3, 2) * m(1, 3) - m(1, 2) * m(3, 3)) + m(3, 1) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3)));
    out(1, 0) = d * (m(1, 2) * (m(2, 0) * m(3, 3) - m(3, 0) * m(2, 3)) + m(2, 2) * (m(3, 0) * m(1, 3) - m(1, 0) * m(3, 3)) + m(3, 2) * (m(1, 0) * m(2, 3) - m(2, 0) * m(1, 3)));
    out(2, 0) = d * (m(1, 3) * (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) + m(2, 3) * (m(3, 0) * m(1, 1) - m(1, 0) * m(3, 1)) + m(3, 3) * (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)));
    out(3, 0) = d * (m(1, 0) * (m(3, 1) * m(2, 2) - m(2, 1) * m(3, 2)) + m(2, 0) * (m(1, 1) * m(3, 2) - m(3, 1) * m(1, 2)) + m(3, 0) * (m(2, 1) * m(1, 2) - m(1, 1) * m(2, 2)));
    out(0, 1) = d * (m(2, 1) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3)) + m(3, 1) * (m(2, 2) * m(0, 3) - m(0, 2) * m(2, 3)) + m(0, 1) * (m(3, 2) * m(2, 3) - m(2, 2) * m(3, 3)));
    out(1, 1) = d * (m(2, 2) * (m(0, 0) * m(3, 3) - m(3, 0) * m(0, 3)) + m(3, 2) * (m(2, 0) * m(0, 3) - m(0, 0) * m(2, 3)) + m(0, 2) * (m(3, 0) * m(2, 3) - m(2, 0) * m(3, 3)));
    out(2, 1) = d * (m(2, 3) * (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) + m(3, 3) * (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) + m(0, 3) * (m(3, 0) * m(2, 1) - m(2, 0) * m(3, 1)));
    out(3, 1) = d * (m(2, 0) * (m(3, 1) * m(0, 2) - m(0, 1) * m(3, 2)) + m(3, 0) * (m(0, 1) * m(2, 2) - m(2, 1) * m(0, 2)) + m(0, 0) * (m(2, 1) * m(3, 2) - m(3, 1) * m(2, 2)));
    out(0, 2) = d * (m(3, 1) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3)) + m(0, 1) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3)) + m(1, 1) * (m(3, 2) * m(0, 3) - m(0, 2) * m(3, 3)));
    out(1, 2) = d * (m(3, 2) * (m(0, 0) * m(1, 3) - m(1, 0) * m(0, 3)) + m(0, 2) * (m(1, 0) * m(3, 3) - m(3, 0) * m(1, 3)) + m(1, 2) * (m(3, 0) * m(0, 3) - m(0, 0) * m(3, 3)));
    out(2, 2) = d * (m(3, 3) * (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) + m(0, 3) * (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) + m(1, 3) * (m(3, 0) * m(0, 1) - m(0, 0) * m(3, 1)));
    out(3, 2) = d * (m(3, 0) * (m(1, 1) * m(0, 2) - m(0, 1) * m(1, 2)) + m(0, 0) * (m(3, 1) * m(1, 2) - m(1, 1) * m(3, 2)) + m(1, 0) * (m(0, 1) * m(3, 2) - m(3, 1) * m(0, 2)));
    out(0, 3) = d * (m(0, 1) * (m(2, 2) * m(1, 3) - m(1, 2) * m(2, 3)) + m(1, 1) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3)) + m(2, 1) * (m(1, 2) * m(0, 3) - m(0, 2) * m(1, 3)));
    out(1, 3) = d * (m(0, 2) * (m(2, 0) * m(1, 3) - m(1, 0) * m(2, 3)) + m(1, 2) * (m(0, 0) * m(2, 3) - m(2, 0) * m(0, 3)) + m(2, 2) * (m(1, 0) * m(0, 3) - m(0, 0) * m(1, 3)));
    out(2, 3) = d * (m(0, 3) * (m(2, 0) * m(1, 1) - m(1, 0) * m(2, 1)) + m(1, 3) * (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) + m(2, 3) * (m(1, 0) * m(0, 1) - m(0, 0) * m(1, 1)));
    out(3, 3) = d * (m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) + m(1, 0) * (m(2, 1) * m(0, 2) - m(0, 1) * m(2, 2)) + m(2, 0) * (m(0, 1) * m(1, 2) - m(1, 1) * m(0, 2)));

    return true;
}
inline bool Matrix4::Inverse()
{
    Matrix4 temp;
    bool can = GetInverse(temp);
    if (can)
    {
        *this = temp;
    }
    return can;
}

inline bool Matrix4::Decomposition(Vector3& position, Vector3& scale, Vector3& orientation) const
{
    //if(_data[3][0] == 0 && _data[3][1] == 0 && _data[3][2] == 0 && _data[3][3] == 1)
    //{
    //	return false;
    //}
    //else
    {
        /*Matrix3 mat3(_data[0][0], _data[0][1], _data[0][2], 
			_data[1][0], _data[1][1], _data[1][2], 
			_data[2][0], _data[2][1], _data[2][2]);
		Matrix3 matQ;
		Vector3 vecU;
		mat3.Decomposition(matQ, scale, vecU);*/

        //as for now we have only uniform scale - no matrix orthoganalization , check it
        /*DVASSERT((fabs(_00*_10+_01*_11+_02*_12)<0.001f) && "Only orthoganal basis accepted");
		DVASSERT((fabs(_10*_20+_11*_21+_12*_22)<0.001f) && "Only orthoganal basis accepted");
		DVASSERT((fabs(_20*_00+_21*_01+_22*_02)<0.001f) && "Only orthoganal basis accepted");*/

        scale.x = std::sqrt(_00 * _00 + _01 * _01 + _02 * _02);
        scale.y = std::sqrt(_10 * _10 + _11 * _11 + _12 * _12);
        scale.z = std::sqrt(_20 * _20 + _21 * _21 + _22 * _22);

        orientation.x = std::atan2(_21, _22);
        orientation.y = std::atan2(-_20, std::sqrt(_21 * _21 + _22 * _22));
        orientation.z = std::atan2(_10, _00);

        position = Vector3(_data[0][3], _data[1][3], _data[2][3]);

        return true;
    }

    return false;
}

inline Matrix4 Matrix4::operator*(const Matrix4& m) const
{
//matrixMultiplicationCounter++;
	
#ifdef __DAVAENGINE_ARM_7__
    Matrix4 res;
    NEON_Matrix4Mul(this->data, m.data, res.data);
    return res;
#else
    return Matrix4(_00 * m._00 + _01 * m._10 + _02 * m._20 + _03 * m._30,
                   _00 * m._01 + _01 * m._11 + _02 * m._21 + _03 * m._31,
                   _00 * m._02 + _01 * m._12 + _02 * m._22 + _03 * m._32,
                   _00 * m._03 + _01 * m._13 + _02 * m._23 + _03 * m._33,

                   _10 * m._00 + _11 * m._10 + _12 * m._20 + _13 * m._30,
                   _10 * m._01 + _11 * m._11 + _12 * m._21 + _13 * m._31,
                   _10 * m._02 + _11 * m._12 + _12 * m._22 + _13 * m._32,
                   _10 * m._03 + _11 * m._13 + _12 * m._23 + _13 * m._33,

                   _20 * m._00 + _21 * m._10 + _22 * m._20 + _23 * m._30,
                   _20 * m._01 + _21 * m._11 + _22 * m._21 + _23 * m._31,
                   _20 * m._02 + _21 * m._12 + _22 * m._22 + _23 * m._32,
                   _20 * m._03 + _21 * m._13 + _22 * m._23 + _23 * m._33,

                   _30 * m._00 + _31 * m._10 + _32 * m._20 + _33 * m._30,
                   _30 * m._01 + _31 * m._11 + _32 * m._21 + _33 * m._31,
                   _30 * m._02 + _31 * m._12 + _32 * m._22 + _33 * m._32,
                   _30 * m._03 + _31 * m._13 + _32 * m._23 + _33 * m._33); 
#endif
}

inline const Matrix4& Matrix4::operator*=(const Matrix4& m)
{
    *this = (*this) * m;
    return *this;
}

inline void Matrix4::CreateRotation(const Vector3& r, float32 angleInRadians)
{
    float32 cosA = std::cos(angleInRadians);
    float32 sinA = std::sin(angleInRadians);
    Identity();
    _data[0][0] = cosA + (1 - cosA) * r.x * r.x;
    _data[0][1] = (1 - cosA) * r.x * r.y - r.z * sinA;
    _data[0][2] = (1 - cosA) * r.x * r.z + r.y * sinA;

    _data[1][0] = (1 - cosA) * r.x * r.y + r.z * sinA;
    _data[1][1] = cosA + (1 - cosA) * r.y * r.y;
    _data[1][2] = (1 - cosA) * r.y * r.z - r.x * sinA;

    _data[2][0] = (1 - cosA) * r.x * r.z - r.y * sinA;
    _data[2][1] = (1 - cosA) * r.y * r.z + r.x * sinA;
    _data[2][2] = cosA + (1 - cosA) * r.z * r.z;
}

inline void Matrix4::CreateTranslation(const Vector3& _v)
{
    Identity();
    SetTranslationVector(_v);
}

inline Vector3 Matrix4::GetTranslationVector() const
{
    return Vector3(_30, _31, _32);
}

inline void Matrix4::SetTranslationVector(const Vector3& _v)
{
    _30 = _v.x;
    _31 = _v.y;
    _32 = _v.z;
}

inline Vector3 Matrix4::GetScaleVector() const
{
    Vector3 xAxis(_00, _01, _02);
    Vector3 yAxis(_10, _11, _12);
    Vector3 zAxis(_20, _21, _22);
    return Vector3(xAxis.Length(), yAxis.Length(), zAxis.Length());
}

inline void Matrix4::CreateScale(const Vector3& _v)
{
    Identity();
    _00 = _v.x;
    _11 = _v.y;
    _22 = _v.z;
}

inline Matrix4 Matrix4::MakeTranslation(const Vector3& translationVector)
{
    Matrix4 result;
    result.CreateTranslation(translationVector);
    return result;
}

inline Matrix4 Matrix4::MakeRotation(const Vector3& axis, float32 angleInRadians)
{
    Matrix4 result;
    result.CreateRotation(axis, angleInRadians);
    return result;
}

inline Matrix4 Matrix4::MakeScale(const Vector3& scaleVector)
{
    Matrix4 result;
    result.CreateScale(scaleVector);
    return result;
}

//! Comparison operators
inline bool Matrix4::operator==(const Matrix4& _m) const
{
    // Check translation first
    if (!FLOAT_EQUAL(_30, _m._30))
        return false;
    if (!FLOAT_EQUAL(_31, _m._31))
        return false;
    if (!FLOAT_EQUAL(_32, _m._32))
        return false;

    if (!FLOAT_EQUAL(_00, _m._00))
        return false;
    if (!FLOAT_EQUAL(_11, _m._11))
        return false;
    if (!FLOAT_EQUAL(_22, _m._22))
        return false;

    //    if (!FLOAT_EQUAL(_00, _m._00))return false;
    if (!FLOAT_EQUAL(_01, _m._01))
        return false;
    if (!FLOAT_EQUAL(_02, _m._02))
        return false;
    if (!FLOAT_EQUAL(_03, _m._03))
        return false;

    if (!FLOAT_EQUAL(_10, _m._10))
        return false;
    //    if (!FLOAT_EQUAL(_11, _m._11))return false;
    if (!FLOAT_EQUAL(_12, _m._12))
        return false;
    if (!FLOAT_EQUAL(_13, _m._13))
        return false;

    if (!FLOAT_EQUAL(_20, _m._20))
        return false;
    if (!FLOAT_EQUAL(_21, _m._21))
        return false;
    //    if (!FLOAT_EQUAL(_22, _m._22))return false;
    if (!FLOAT_EQUAL(_23, _m._23))
        return false;

    //    if (!FLOAT_EQUAL(_30, _m._30))return false;
    //    if (!FLOAT_EQUAL(_31, _m._31))return false;
    //    if (!FLOAT_EQUAL(_32, _m._32))return false;
    if (!FLOAT_EQUAL(_33, _m._33))
        return false;

    return true;
}

inline bool Matrix4::operator!=(const Matrix4& _m) const
{
    return !Matrix4::operator==(_m);
}

}; // end of namespace DAVA
