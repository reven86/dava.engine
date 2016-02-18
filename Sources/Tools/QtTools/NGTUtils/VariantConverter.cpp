/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "VariantConverter.h"

#include "Base/FastName.h"
#include "Math/Color.h"
#include "Math/AABBox3.h"
#include "Math/Vector.h"
#include "Math/Matrix2.h"
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/KeyedArchive.h"

#include "Debug/DVAssert.h"

#include "wg_types/vector2.hpp"
#include "wg_types/vector3.hpp"
#include "wg_types/vector4.hpp"

namespace DAVA
{
namespace VCLocal
{
static const uint8 maxChannelValue = 255;
}

template <typename T>
VariantType VtoDV(Variant const& v)
{
    T value = T();
    DVVERIFY(v.tryCast(value));

    return VariantType(value);
}

template <>
VariantType VtoDV<void>(Variant const& /*v*/)
{
    return VariantType();
}

template <>
VariantType VtoDV<FastName>(Variant const& v)
{
    String strValue;
    DVVERIFY(v.tryCast(strValue));
    return VariantType(FastName(strValue));
}

template <>
VariantType VtoDV<Vector2>(Variant const& v)
{
    ::Vector2 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(Vector2(value.x, value.y));
}

template <>
VariantType VtoDV<Vector3>(Variant const& v)
{
    ::Vector3 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(Vector3(value.x, value.y, value.z));
}

template <>
VariantType VtoDV<Vector4>(Variant const& v)
{
    ::Vector4 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(Vector4(value.x, value.y, value.z, value.w));
}

template <>
VariantType VtoDV<Color>(Variant const& v)
{
    ::Vector4 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(Color(value.x / VCLocal::maxChannelValue, value.y / VCLocal::maxChannelValue,
                             value.z / VCLocal::maxChannelValue, value.w / VCLocal::maxChannelValue));
}

template <>
VariantType VtoDV<FilePath>(Variant const& v)
{
    String filePath;
    DVVERIFY(v.tryCast(filePath));
    return VariantType(FilePath(filePath));
}

template <>
VariantType VtoDV<float64>(Variant const& v)
{
    float64 value;
    DVVERIFY(v.tryCast(value));
    return VariantType(value);
}

Variant DVtoV_void(VariantType const& v)
{
    return Variant();
}
Variant DVtoV_bool(VariantType const& v)
{
    return Variant(v.AsBool());
}
Variant DVtoV_int32(VariantType const& v)
{
    return Variant(v.AsInt32());
}
Variant DVtoV_float(VariantType const& v)
{
    return Variant(v.AsFloat());
}
Variant DVtoV_string(VariantType const& v)
{
    return Variant(v.AsString());
}
Variant DVtoV_wideString(VariantType const& v)
{
    return Variant(v.AsWideString());
}
Variant DVtoV_int64(VariantType const& v)
{
    return Variant(v.AsInt64());
}
Variant DVtoV_uint32(VariantType const& v)
{
    return Variant(v.AsUInt32());
}
Variant DVtoV_uint64(VariantType const& v)
{
    return Variant(v.AsUInt64());
}
Variant DVtoV_vector2(VariantType const& v)
{
    Vector2 davaVec = v.AsVector2();
    return Variant(::Vector2(davaVec.x, davaVec.y));
}
Variant DVtoV_vector3(VariantType const& v)
{
    Vector3 davaVec = v.AsVector3();
    return Variant(::Vector3(davaVec.x, davaVec.y, davaVec.z));
}
Variant DVtoV_vector4(VariantType const& v)
{
    Vector4 davaVec = v.AsVector4();
    return Variant(::Vector4(davaVec.x, davaVec.y, davaVec.z, davaVec.w));
}
Variant DVtoV_matrix2(VariantType const& v)
{
    const Matrix2 & m = v.AsMatrix2();
    StringStream ss;
    ss.precision(7);
    ss << "[ " << m._00 << "; " << m._01 << " ]\n[ "
               << m._10 << "; " << m._11 << " ]";
    return Variant(ss.str());
}
Variant DVtoV_matrix3(VariantType const& v)
{
    const Matrix4 & m = v.AsMatrix4();
    StringStream ss;
    ss.precision(7);
    ss << "[ " << m._00 << "; " << m._01 << "; " << m._02 << " ]\n[ "
        << m._10 << "; " << m._11 << "; " << m._12 << " ]\n[ "
        << m._20 << "; " << m._21 << "; " << m._22 << " ]";
    return Variant(ss.str());
}
Variant DVtoV_matrix4(VariantType const& v)
{
    const Matrix4 & m = v.AsMatrix4();
    StringStream ss;
    ss.precision(7);
    ss << "[ " << m._00 << "; " << m._01 << "; " << m._02 << ";" << m._03 << " ]\n["
        << m._10 << "; " << m._11 << "; " << m._12 << ";" << m._13 << " ]\n[ "
        << m._20 << "; " << m._21 << "; " << m._22 << ";" << m._23 << " ]\n[ "
        << m._30 << "; " << m._31 << "; " << m._32 << ";" << m._33 << " ]";
    return Variant(ss.str());
}
Variant DVtoV_color(VariantType const& v)
{
    Color davaVec = v.AsColor();
    return Variant(::Vector4(davaVec.r * VCLocal::maxChannelValue, davaVec.g * VCLocal::maxChannelValue,
                             davaVec.b * VCLocal::maxChannelValue, davaVec.a * VCLocal::maxChannelValue));
}
Variant DVtoV_fastName(VariantType const& v)
{
    return Variant(v.AsFastName().c_str());
}
Variant DVtoV_aabbox3(VariantType const& v)
{
    const AABBox3 & m = v.AsAABBox3();
    StringStream ss;
    ss.precision(7);
    ss << "[ " << m.min.x << "; " << m.min.y << "; " << m.min.z << " ]\n[ "
               << m.max.x << "; " << m.max.y << "; " << m.max.z << " ]";
    return Variant(ss.str());
}

Variant DVtoV_filePath(VariantType const& v)
{
    return Variant(v.AsFilePath().GetAbsolutePathname());
}

Variant DVtoV_float64(VariantType const& v)
{
    return Variant(v.AsFloat64());
}

VariantConverter::VariantConverter()
{
    using namespace DAVA;
    using namespace std;
    using namespace std::placeholders;

    convertFunctions[VariantType::TYPE_NONE] = { bind(&VtoDV<void>, _1), bind(&DVtoV_void, _1) };
    convertFunctions[VariantType::TYPE_BOOLEAN] = { bind(&VtoDV<bool>, _1), bind(&DVtoV_bool, _1) };
    convertFunctions[VariantType::TYPE_INT32] = { bind(&VtoDV<int32>, _1), bind(&DVtoV_int32, _1) };
    convertFunctions[VariantType::TYPE_FLOAT] = { bind(&VtoDV<float32>, _1), bind(&DVtoV_float, _1) };
    convertFunctions[VariantType::TYPE_STRING] = { bind(&VtoDV<String>, _1), bind(&DVtoV_string, _1) };
    convertFunctions[VariantType::TYPE_WIDE_STRING] = { bind(&VtoDV<WideString>, _1), bind(&DVtoV_wideString, _1) };
    convertFunctions[VariantType::TYPE_UINT32] = { bind(&VtoDV<uint32>, _1), bind(&DVtoV_uint32, _1) };
    convertFunctions[VariantType::TYPE_INT64] = { bind(&VtoDV<int64>, _1), bind(&DVtoV_int64, _1) };
    convertFunctions[VariantType::TYPE_UINT64] = { bind(&VtoDV<uint64>, _1), bind(&DVtoV_uint64, _1) };
    convertFunctions[VariantType::TYPE_VECTOR2] = { bind(&VtoDV<Vector2>, _1), bind(&DVtoV_vector2, _1) };
    convertFunctions[VariantType::TYPE_VECTOR3] = { bind(&VtoDV<Vector3>, _1), bind(&DVtoV_vector3, _1) };
    convertFunctions[VariantType::TYPE_VECTOR4] = { bind(&VtoDV<Vector4>, _1), bind(&DVtoV_vector4, _1) };
    convertFunctions[VariantType::TYPE_MATRIX2] = { bind(&VtoDV<Matrix2>, _1), bind(&DVtoV_matrix2, _1) };
    convertFunctions[VariantType::TYPE_MATRIX3] = { bind(&VtoDV<Matrix3>, _1), bind(&DVtoV_matrix3, _1) };
    convertFunctions[VariantType::TYPE_MATRIX4] = { bind(&VtoDV<Matrix4>, _1), bind(&DVtoV_matrix4, _1) };
    convertFunctions[VariantType::TYPE_COLOR] = { bind(&VtoDV<Color>, _1), bind(&DVtoV_color, _1) };
    convertFunctions[VariantType::TYPE_FASTNAME] = { bind(&VtoDV<FastName>, _1), bind(&DVtoV_fastName, _1) };
    convertFunctions[VariantType::TYPE_AABBOX3] = { bind(&VtoDV<AABBox3>, _1), bind(&DVtoV_aabbox3, _1) };
    convertFunctions[VariantType::TYPE_FILEPATH] = { bind(&VtoDV<FilePath>, _1), bind(&DVtoV_filePath, _1) };
    convertFunctions[VariantType::TYPE_FLOAT64] = { bind(&VtoDV<float64>, _1), bind(&DVtoV_float64, _1) };

#ifdef __DAVAENGINE_DEBUG__
    for (int i = 0; i < VariantType::TYPES_COUNT; ++i)
    {
        VariantType::eVariantType type = static_cast <VariantType::eVariantType>(i);
        if (type == VariantType::TYPE_BYTE_ARRAY || type == VariantType::TYPE_KEYED_ARCHIVE)
        {
            continue;
        }

        ConvertNode const & node = convertFunctions[i];
        DVASSERT(node.dvToVFn);
        DVASSERT(node.vToDvFn);
    }
#endif
}

VariantType VariantConverter::Convert(Variant const& v, MetaInfo const* info) const
{
    VariantType::eVariantType davaType = VariantType::TYPE_NONE;
    for (VariantType::PairTypeName const& type : VariantType::variantNamesMap)
    {
        if (type.variantMeta == info)
        {
            davaType = type.variantType;
            break;
        }
    }
    DVASSERT(davaType != VariantType::TYPE_BYTE_ARRAY &&
             davaType != VariantType::TYPE_KEYED_ARCHIVE);

    return convertFunctions[davaType].vToDvFn(v);
}

Variant VariantConverter::Convert(const VariantType& value) const
{
    VariantType::eVariantType davaType = value.GetType();
    DVASSERT(davaType != VariantType::TYPE_BYTE_ARRAY &&
             davaType != VariantType::TYPE_KEYED_ARCHIVE);
    return convertFunctions[davaType].dvToVFn(value);
}
}