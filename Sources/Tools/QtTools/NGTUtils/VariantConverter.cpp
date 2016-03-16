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

namespace NGTLayer
{
namespace VCLocal
{
static const DAVA::uint8 maxChannelValue = 255;

template <typename T>
DAVA::VariantType VtoDV(Variant const& v)
{
    T value = T();
    DVVERIFY(v.tryCast(value));

    return DAVA::VariantType(value);
}

template <>
DAVA::VariantType VtoDV<void>(Variant const& /*v*/)
{
    return DAVA::VariantType();
}

template <>
DAVA::VariantType VtoDV<DAVA::FastName>(Variant const& v)
{
    DAVA::String strValue;
    DVVERIFY(v.tryCast(strValue));
    return DAVA::VariantType(DAVA::FastName(strValue));
}

template <>
DAVA::VariantType VtoDV<DAVA::Vector2>(Variant const& v)
{
    ::Vector2 value;
    DVVERIFY(v.tryCast(value));
    return DAVA::VariantType(DAVA::Vector2(value.x, value.y));
}

template <>
DAVA::VariantType VtoDV<DAVA::Vector3>(Variant const& v)
{
    ::Vector3 value;
    DVVERIFY(v.tryCast(value));
    return DAVA::VariantType(DAVA::Vector3(value.x, value.y, value.z));
}

template <>
DAVA::VariantType VtoDV<DAVA::Vector4>(Variant const& v)
{
    ::Vector4 value;
    DVVERIFY(v.tryCast(value));
    return DAVA::VariantType(DAVA::Vector4(value.x, value.y, value.z, value.w));
}

template <>
DAVA::VariantType VtoDV<DAVA::Color>(Variant const& v)
{
    ::Vector4 value;
    DVVERIFY(v.tryCast(value));
    return DAVA::VariantType(DAVA::Color(value.x / VCLocal::maxChannelValue, value.y / VCLocal::maxChannelValue,
                                         value.z / VCLocal::maxChannelValue, value.w / VCLocal::maxChannelValue));
}

template <>
DAVA::VariantType VtoDV<DAVA::FilePath>(Variant const& v)
{
    DAVA::String filePath;
    DVVERIFY(v.tryCast(filePath));
    return DAVA::VariantType(DAVA::FilePath(filePath));
}

Variant DVtoV_void(DAVA::VariantType const& v)
{
    return Variant();
}
Variant DVtoV_bool(DAVA::VariantType const& v)
{
    return Variant(v.AsBool());
}
Variant DVtoV_int32(DAVA::VariantType const& v)
{
    return Variant(v.AsInt32());
}
Variant DVtoV_float(DAVA::VariantType const& v)
{
    return Variant(v.AsFloat());
}
Variant DVtoV_string(DAVA::VariantType const& v)
{
    return Variant(v.AsString());
}
Variant DVtoV_wideString(DAVA::VariantType const& v)
{
    return Variant(v.AsWideString());
}
Variant DVtoV_int64(DAVA::VariantType const& v)
{
    return Variant(v.AsInt64());
}
Variant DVtoV_uint32(DAVA::VariantType const& v)
{
    return Variant(v.AsUInt32());
}
Variant DVtoV_uint64(DAVA::VariantType const& v)
{
    return Variant(v.AsUInt64());
}
Variant DVtoV_vector2(DAVA::VariantType const& v)
{
    DAVA::Vector2 davaVec = v.AsVector2();
    return Variant(::Vector2(davaVec.x, davaVec.y));
}
Variant DVtoV_vector3(DAVA::VariantType const& v)
{
    DAVA::Vector3 davaVec = v.AsVector3();
    return Variant(::Vector3(davaVec.x, davaVec.y, davaVec.z));
}
Variant DVtoV_vector4(DAVA::VariantType const& v)
{
    DAVA::Vector4 davaVec = v.AsVector4();
    return Variant(::Vector4(davaVec.x, davaVec.y, davaVec.z, davaVec.w));
}
Variant DVtoV_matrix2(DAVA::VariantType const& v)
{
    const DAVA::Matrix2& m = v.AsMatrix2();
    DAVA::StringStream ss;
    ss.precision(7);
    ss << "[ " << m._00 << "; " << m._01 << " ]\n[ "
       << m._10 << "; " << m._11 << " ]";
    return Variant(ss.str());
}
Variant DVtoV_matrix3(DAVA::VariantType const& v)
{
    const DAVA::Matrix4& m = v.AsMatrix4();
    DAVA::StringStream ss;
    ss.precision(7);
    ss << "[ " << m._00 << "; " << m._01 << "; " << m._02 << " ]\n[ "
        << m._10 << "; " << m._11 << "; " << m._12 << " ]\n[ "
        << m._20 << "; " << m._21 << "; " << m._22 << " ]";
    return Variant(ss.str());
}
Variant DVtoV_matrix4(DAVA::VariantType const& v)
{
    const DAVA::Matrix4& m = v.AsMatrix4();
    DAVA::StringStream ss;
    ss.precision(7);
    ss << "[ " << m._00 << "; " << m._01 << "; " << m._02 << ";" << m._03 << " ]\n["
        << m._10 << "; " << m._11 << "; " << m._12 << ";" << m._13 << " ]\n[ "
        << m._20 << "; " << m._21 << "; " << m._22 << ";" << m._23 << " ]\n[ "
        << m._30 << "; " << m._31 << "; " << m._32 << ";" << m._33 << " ]";
    return Variant(ss.str());
}
Variant DVtoV_color(DAVA::VariantType const& v)
{
    DAVA::Color davaVec = v.AsColor();
    return Variant(::Vector4(davaVec.r * VCLocal::maxChannelValue, davaVec.g * VCLocal::maxChannelValue,
                             davaVec.b * VCLocal::maxChannelValue, davaVec.a * VCLocal::maxChannelValue));
}
Variant DVtoV_fastName(DAVA::VariantType const& v)
{
    return Variant(v.AsFastName().c_str());
}
Variant DVtoV_aabbox3(DAVA::VariantType const& v)
{
    const DAVA::AABBox3& m = v.AsAABBox3();
    DAVA::StringStream ss;
    ss.precision(7);
    ss << "[ " << m.min.x << "; " << m.min.y << "; " << m.min.z << " ]\n[ "
       << m.max.x << "; " << m.max.y << "; " << m.max.z << " ]";
    return Variant(ss.str());
}

Variant DVtoV_filePath(DAVA::VariantType const& v)
{
    return Variant(v.AsFilePath().GetAbsolutePathname());
}

Variant DVtoV_float64(DAVA::VariantType const& v)
{
    return Variant(v.AsFloat64());
}

class Converter
{
public:
    using TVtoDV = DAVA::Function<DAVA::VariantType(Variant const&)>;
    using TDVtoV = DAVA::Function<Variant(DAVA::VariantType const&)>;
    struct ConvertNode
    {
        TVtoDV vToDvFn;
        TDVtoV dvToVFn;
    };

    Converter();

    DAVA::VariantType Convert(Variant const& v, DAVA::MetaInfo const* info);
    Variant Convert(DAVA::VariantType const& value);

private:
    DAVA::Array<ConvertNode, DAVA::VariantType::TYPES_COUNT> convertFunctions;
};

Converter::Converter()
{
    using namespace std;
    using namespace std::placeholders;

    convertFunctions[DAVA::VariantType::TYPE_NONE] = { bind(&VtoDV<void>, _1), bind(&DVtoV_void, _1) };
    convertFunctions[DAVA::VariantType::TYPE_BOOLEAN] = { bind(&VtoDV<bool>, _1), bind(&DVtoV_bool, _1) };
    convertFunctions[DAVA::VariantType::TYPE_INT32] = { bind(&VtoDV<DAVA::int32>, _1), bind(&DVtoV_int32, _1) };
    convertFunctions[DAVA::VariantType::TYPE_FLOAT] = { bind(&VtoDV<DAVA::float32>, _1), bind(&DVtoV_float, _1) };
    convertFunctions[DAVA::VariantType::TYPE_STRING] = { bind(&VtoDV<DAVA::String>, _1), bind(&DVtoV_string, _1) };
    convertFunctions[DAVA::VariantType::TYPE_WIDE_STRING] = { bind(&VtoDV<DAVA::WideString>, _1), bind(&DVtoV_wideString, _1) };
    convertFunctions[DAVA::VariantType::TYPE_UINT32] = { bind(&VtoDV<DAVA::uint32>, _1), bind(&DVtoV_uint32, _1) };
    convertFunctions[DAVA::VariantType::TYPE_INT64] = { bind(&VtoDV<DAVA::int64>, _1), bind(&DVtoV_int64, _1) };
    convertFunctions[DAVA::VariantType::TYPE_UINT64] = { bind(&VtoDV<DAVA::uint64>, _1), bind(&DVtoV_uint64, _1) };
    convertFunctions[DAVA::VariantType::TYPE_VECTOR2] = { bind(&VtoDV<DAVA::Vector2>, _1), bind(&DVtoV_vector2, _1) };
    convertFunctions[DAVA::VariantType::TYPE_VECTOR3] = { bind(&VtoDV<DAVA::Vector3>, _1), bind(&DVtoV_vector3, _1) };
    convertFunctions[DAVA::VariantType::TYPE_VECTOR4] = { bind(&VtoDV<DAVA::Vector4>, _1), bind(&DVtoV_vector4, _1) };
    convertFunctions[DAVA::VariantType::TYPE_MATRIX2] = { bind(&VtoDV<DAVA::Matrix2>, _1), bind(&DVtoV_matrix2, _1) };
    convertFunctions[DAVA::VariantType::TYPE_MATRIX3] = { bind(&VtoDV<DAVA::Matrix3>, _1), bind(&DVtoV_matrix3, _1) };
    convertFunctions[DAVA::VariantType::TYPE_MATRIX4] = { bind(&VtoDV<DAVA::Matrix4>, _1), bind(&DVtoV_matrix4, _1) };
    convertFunctions[DAVA::VariantType::TYPE_COLOR] = { bind(&VtoDV<DAVA::Color>, _1), bind(&DVtoV_color, _1) };
    convertFunctions[DAVA::VariantType::TYPE_FASTNAME] = { bind(&VtoDV<DAVA::FastName>, _1), bind(&DVtoV_fastName, _1) };
    convertFunctions[DAVA::VariantType::TYPE_AABBOX3] = { bind(&VtoDV<DAVA::AABBox3>, _1), bind(&DVtoV_aabbox3, _1) };
    convertFunctions[DAVA::VariantType::TYPE_FILEPATH] = { bind(&VtoDV<DAVA::FilePath>, _1), bind(&DVtoV_filePath, _1) };
    convertFunctions[DAVA::VariantType::TYPE_FLOAT64] = { bind(&VtoDV<DAVA::float64>, _1), bind(&DVtoV_float64, _1) };

#ifdef __DAVAENGINE_DEBUG__
    for (int i = 0; i < DAVA::VariantType::TYPES_COUNT; ++i)
    {
        DAVA::VariantType::eVariantType type = static_cast<DAVA::VariantType::eVariantType>(i);
        if (type == DAVA::VariantType::TYPE_BYTE_ARRAY || type == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
        {
            continue;
        }

        ConvertNode const& node = convertFunctions[i];
        DVASSERT(node.dvToVFn);
        DVASSERT(node.vToDvFn);
    }
#endif
}

inline DAVA::VariantType Converter::Convert(Variant const& v, DAVA::MetaInfo const* info)
{
    DAVA::VariantType::eVariantType davaType = DAVA::VariantType::TYPE_NONE;
    for (DAVA::VariantType::PairTypeName const& type : DAVA::VariantType::variantNamesMap)
    {
        if (type.variantMeta == info)
        {
            davaType = type.variantType;
            break;
        }
    }
    DVASSERT(davaType != DAVA::VariantType::TYPE_BYTE_ARRAY &&
             davaType != DAVA::VariantType::TYPE_KEYED_ARCHIVE);

    return convertFunctions[davaType].vToDvFn(v);
}

inline Variant Converter::Convert(DAVA::VariantType const& value)
{
    using namespace VCLocal;

    DAVA::VariantType::eVariantType davaType = value.GetType();
    DVASSERT(davaType != DAVA::VariantType::TYPE_BYTE_ARRAY &&
             davaType != DAVA::VariantType::TYPE_KEYED_ARCHIVE);
    return convertFunctions[davaType].dvToVFn(value);
}

Converter g_converter;

} // namespace VCLocal

namespace VariantConverter
{
DAVA::VariantType Convert(Variant const& v, DAVA::MetaInfo const* info)
{
    return VCLocal::g_converter.Convert(v, info);
}

Variant Convert(const DAVA::VariantType& value)
{
    return VCLocal::g_converter.Convert(value);
}
} // namespace VariantConverter
} // namespace NGTLayer