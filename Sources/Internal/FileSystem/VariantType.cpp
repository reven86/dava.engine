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


#include "FileSystem/File.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Math/MathConstants.h"
#include "Math/Math2D.h"
#include "Math/Vector.h"
#include "Math/Matrix2.h"
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"
#include "Utils/Utils.h"
#include "Base/Meta.h"
#include "Math/Color.h"
#include "Base/FastName.h"
#include "Math/AABBox3.h"

namespace DAVA
{

const String VariantType::TYPENAME_UNKNOWN = "unknown";
const String VariantType::TYPENAME_BOOLEAN = "bool";
const String VariantType::TYPENAME_INT32   = "int32";
const String VariantType::TYPENAME_UINT32  = "uint32";
const String VariantType::TYPENAME_INT64   = "int64";
const String VariantType::TYPENAME_UINT64  = "uint64";
    
const String VariantType::TYPENAME_FLOAT   = "float";
const String VariantType::TYPENAME_FLOAT64 = "float64";
const String VariantType::TYPENAME_STRING  = "string";
const String VariantType::TYPENAME_WIDESTRING = "wideString";
const String VariantType::TYPENAME_BYTE_ARRAY = "byteArray";
const String VariantType::TYPENAME_KEYED_ARCHIVE = "keyedArchive";
const String VariantType::TYPENAME_VECTOR2 = "Vector2";
const String VariantType::TYPENAME_VECTOR3 = "Vector3";
const String VariantType::TYPENAME_VECTOR4 = "Vector4";
const String VariantType::TYPENAME_MATRIX2 = "Matrix2";
const String VariantType::TYPENAME_MATRIX3 = "Matrix3";
const String VariantType::TYPENAME_MATRIX4 = "Matrix4";
const String VariantType::TYPENAME_COLOR   = "Color";
const String VariantType::TYPENAME_FASTNAME= "FastName";
const String VariantType::TYPENAME_AABBOX3 = "AABBox3";
const String VariantType::TYPENAME_FILEPATH = "FilePath";
    
const Array<VariantType::PairTypeName, VariantType::TYPES_COUNT> VariantType::variantNamesMap =
{{
    VariantType::PairTypeName(VariantType::TYPE_NONE,          TYPENAME_UNKNOWN,		nullptr),
    VariantType::PairTypeName(VariantType::TYPE_BOOLEAN,       TYPENAME_BOOLEAN,		MetaInfo::Instance<bool>()),
    VariantType::PairTypeName(VariantType::TYPE_INT32,         TYPENAME_INT32,			MetaInfo::Instance<int32>()),
    VariantType::PairTypeName(VariantType::TYPE_FLOAT,         TYPENAME_FLOAT,			MetaInfo::Instance<float32>()),
    VariantType::PairTypeName(VariantType::TYPE_STRING,        TYPENAME_STRING,			MetaInfo::Instance<String>()),
    VariantType::PairTypeName(VariantType::TYPE_WIDE_STRING,   TYPENAME_WIDESTRING,		MetaInfo::Instance<WideString>()),
    VariantType::PairTypeName(VariantType::TYPE_BYTE_ARRAY,    TYPENAME_BYTE_ARRAY,		MetaInfo::Instance<Vector<uint8> >()),
    VariantType::PairTypeName(VariantType::TYPE_UINT32,        TYPENAME_UINT32,			MetaInfo::Instance<uint32>()),
    VariantType::PairTypeName(VariantType::TYPE_KEYED_ARCHIVE, TYPENAME_KEYED_ARCHIVE,	MetaInfo::Instance<KeyedArchive *>()),
    VariantType::PairTypeName(VariantType::TYPE_INT64,         TYPENAME_INT64,			MetaInfo::Instance<int64>()),
    VariantType::PairTypeName(VariantType::TYPE_UINT64,        TYPENAME_UINT64,			MetaInfo::Instance<uint64>()),
    VariantType::PairTypeName(VariantType::TYPE_VECTOR2,       TYPENAME_VECTOR2,		MetaInfo::Instance<Vector2>()),
    VariantType::PairTypeName(VariantType::TYPE_VECTOR3,       TYPENAME_VECTOR3,		MetaInfo::Instance<Vector3>()),
    VariantType::PairTypeName(VariantType::TYPE_VECTOR4,       TYPENAME_VECTOR4,		MetaInfo::Instance<Vector4>()),
    VariantType::PairTypeName(VariantType::TYPE_MATRIX2,       TYPENAME_MATRIX2,		MetaInfo::Instance<Matrix2>()),
    VariantType::PairTypeName(VariantType::TYPE_MATRIX3,       TYPENAME_MATRIX3,		MetaInfo::Instance<Matrix3>()),
    VariantType::PairTypeName(VariantType::TYPE_MATRIX4,       TYPENAME_MATRIX4,		MetaInfo::Instance<Matrix4>()),
	VariantType::PairTypeName(VariantType::TYPE_COLOR,         TYPENAME_COLOR,          MetaInfo::Instance<Color>()),
	VariantType::PairTypeName(VariantType::TYPE_FASTNAME,      TYPENAME_FASTNAME,       MetaInfo::Instance<FastName>()),
	VariantType::PairTypeName(VariantType::TYPE_AABBOX3,       TYPENAME_AABBOX3,        MetaInfo::Instance<AABBox3>()),
	VariantType::PairTypeName(VariantType::TYPE_FILEPATH,      TYPENAME_FILEPATH,       MetaInfo::Instance<FilePath>()),
    VariantType::PairTypeName(VariantType::TYPE_FLOAT64,       TYPENAME_FLOAT64,        MetaInfo::Instance<float64>())
}};

VariantType::VariantType()
    : type(TYPE_NONE)
    , pointerValue(nullptr)
{
}

VariantType::VariantType(const VariantType &var) : pointerValue(nullptr)
{
	SetVariant(var);
}

VariantType::VariantType(bool value) : pointerValue(nullptr)
{
	SetBool(value);
}

VariantType::VariantType(int32 value) : pointerValue(nullptr)
{
	SetInt32(value);
}

VariantType::VariantType(uint32 value) : pointerValue(nullptr)
{
	SetUInt32(value);
}

VariantType::VariantType(float32 value) : pointerValue(nullptr)
{
	SetFloat(value);
}

VariantType::VariantType(float64 value) : pointerValue(nullptr)
{
    SetFloat64(value);
}

VariantType::VariantType(const String & value) : pointerValue(nullptr)
{
	SetString(value);
}

VariantType::VariantType(const WideString & value) : pointerValue(nullptr)
{
	SetWideString(value);
}

VariantType::VariantType(const uint8 *_array, int32 arraySizeInBytes) : pointerValue(nullptr)
{
	SetByteArray(_array, arraySizeInBytes);
}

VariantType::VariantType(KeyedArchive *archive) : pointerValue(nullptr)
{
	SetKeyedArchive(archive);
}

VariantType::VariantType(const int64 & value) : pointerValue(nullptr)
{
	SetInt64(value);
}

VariantType::VariantType(const uint64 & value) : pointerValue(nullptr)
{
	SetUInt64(value);
}

VariantType::VariantType(const Vector2 & value) : pointerValue(nullptr)
{
	SetVector2(value);
}

VariantType::VariantType(const Vector3 & value) : pointerValue(nullptr)
{
	SetVector3(value);
}

VariantType::VariantType(const Vector4 & value) : pointerValue(nullptr)
{
	SetVector4(value);
}

VariantType::VariantType(const Matrix2 & value) : pointerValue(nullptr)
{
	SetMatrix2(value);
}

VariantType::VariantType(const Matrix3 & value) : pointerValue(nullptr)
{
	SetMatrix3(value);
}

VariantType::VariantType(const Matrix4 & value) : pointerValue(nullptr)
{
	SetMatrix4(value);
}

VariantType::VariantType(const Color & value) : pointerValue(nullptr)
{
	SetColor(value);
}

VariantType::VariantType(const FastName & value) : pointerValue(nullptr)
{
	SetFastName(value);
}

VariantType::VariantType(const AABBox3 & value) : pointerValue(nullptr)
{
	SetAABBox3(value);
}

VariantType::VariantType(const FilePath & value) : pointerValue(nullptr)
{
	SetFilePath(value);
}

VariantType::~VariantType()
{
    ReleasePointer();
}

const String &VariantType::GetTypeName() const
{
    DVASSERT(type >= 0 && type < TYPES_COUNT);
    return variantNamesMap[type].variantName;
}

void VariantType::SetBool(bool value)
{
    ReleasePointer();
	type = TYPE_BOOLEAN;
	boolValue = value;
}

void VariantType::SetInt32(int32 value)
{
    ReleasePointer();
	type = TYPE_INT32;
	int32Value = value;
}

void VariantType::SetUInt32(uint32 value)
{
    ReleasePointer();
    type = TYPE_UINT32;
    uint32Value = value;
}

void VariantType::SetFloat(float32 value)
{
    ReleasePointer();
	type = TYPE_FLOAT;
	floatValue = value;
}

void VariantType::SetFloat64(float64 value)
{
    ReleasePointer();
    type = TYPE_FLOAT64;
    float64Value = value;
}

void VariantType::SetString(const String & value)
{
    ReleasePointer();
	type = TYPE_STRING;
	stringValue = new String(value);
}
void VariantType::SetWideString(const WideString & value)
{
    ReleasePointer();
	type = TYPE_WIDE_STRING;
	wideStringValue = new WideString(value);
}

void VariantType::SetByteArray(const uint8 *array, int32 arraySizeInBytes)
{
    ReleasePointer();
	type = TYPE_BYTE_ARRAY;
    auto vec = new Vector < uint8 >(array, array + arraySizeInBytes);
    pointerValue = static_cast<void*>(vec);
}

void VariantType::SetKeyedArchive(KeyedArchive *archive)
{
    ReleasePointer();
    type = TYPE_KEYED_ARCHIVE;
    if(archive)
    {
        pointerValue = new KeyedArchive(*archive);
    }
    else
    {
        pointerValue = new KeyedArchive();
    }
}
    
void VariantType::SetInt64(const int64 & value)
{
    ReleasePointer();
    type = TYPE_INT64;
    int64Value = value;
}

void VariantType::SetUInt64(const uint64 & value)
{
    ReleasePointer();
    type = TYPE_UINT64;
    uint64Value = value;
}

void VariantType::SetVector2(const Vector2 & value)
{
    ReleasePointer();
    type = TYPE_VECTOR2;
    vector2Value = new Vector2(value);
}

void VariantType::SetVector3(const Vector3 & value)
{
    ReleasePointer();
    type = TYPE_VECTOR3;
    vector3Value = new Vector3(value);
}

void VariantType::SetVector4(const Vector4 & value)
{
    ReleasePointer();
    type = TYPE_VECTOR4;
    vector4Value = new Vector4(value);
}

void VariantType::SetMatrix2(const Matrix2 & value)
{
    ReleasePointer();
    type = TYPE_MATRIX2;
    matrix2Value = new Matrix2(value);
}
void VariantType::SetMatrix3(const Matrix3 & value)
{
    ReleasePointer();
    type = TYPE_MATRIX3;
    matrix3Value = new Matrix3(value);
}
void VariantType::SetMatrix4(const Matrix4 & value)
{
    ReleasePointer();
    type = TYPE_MATRIX4;
    matrix4Value = new Matrix4(value);
}

void VariantType::SetColor(const DAVA::Color &value)
{
    ReleasePointer();
    type = TYPE_COLOR;
    colorValue = new Color(value);
}
    
void VariantType::SetFastName(const DAVA::FastName &value)
{
    ReleasePointer();
    type = TYPE_FASTNAME;
    
    fastnameValue = new FastName(value);
}
    
void VariantType::SetAABBox3(const DAVA::AABBox3 &value)
{
	ReleasePointer();
	type = TYPE_AABBOX3;

	aabbox3 = new AABBox3(value);
}

void VariantType::SetFilePath(const FilePath & value)
{
	ReleasePointer();
	type = TYPE_FILEPATH;

	filepathValue = new FilePath(value);
}

void VariantType::SetVariant(const VariantType& var)
{
    if (this == &var)
    {
        return;
    }
    
	type = TYPE_NONE;

	switch(var.type)
	{
	case TYPE_BOOLEAN:
		{
			SetBool(var.boolValue);
		}
		break;
	case TYPE_INT32:
		{
			SetInt32(var.int32Value);
		}
		break;	
	case TYPE_UINT32:
		{
			SetUInt32(var.uint32Value);
		}
		break;	
	case TYPE_FLOAT:
		{
			SetFloat(var.floatValue);
		}
		break;	
    case TYPE_FLOAT64:
        {
            SetFloat64(var.float64Value);
        }
        break;
	case TYPE_STRING:
		{
			SetString(var.AsString());
		}
		break;	
	case TYPE_WIDE_STRING:
		{ 
			SetWideString(var.AsWideString());
		}
		break;
	case TYPE_BYTE_ARRAY:
		{
			Vector<uint8> *ar = (Vector<uint8>*) var.pointerValue;
			SetByteArray(ar->data(), static_cast<int32>(ar->size()));
		}
		break;
	case TYPE_KEYED_ARCHIVE:
		{
			SetKeyedArchive(var.AsKeyedArchive());
		}
		break;
	case TYPE_INT64:
		{
			SetInt64(var.AsInt64());
		}
		break;
	case TYPE_UINT64:
		{
			SetUInt64(var.AsUInt64());
		}
		break;
	case TYPE_VECTOR2:
		{
			SetVector2(var.AsVector2());
		}
		break;
	case TYPE_VECTOR3:
		{
			SetVector3(var.AsVector3());
		}
		break;
	case TYPE_VECTOR4:
		{
			SetVector4(var.AsVector4());
		}
		break;
	case TYPE_MATRIX2:
		{
			SetMatrix2(var.AsMatrix2());
		}
		break;
	case TYPE_MATRIX3:
		{
			SetMatrix3(var.AsMatrix3());
		}
		break;
	case TYPE_MATRIX4:
		{
			SetMatrix4(var.AsMatrix4());
		}
		break;
    case  TYPE_COLOR:
		{
			SetColor(var.AsColor());
		}
        break;
    case TYPE_FASTNAME:
        {
            SetFastName(var.AsFastName());
        }
        break;
	case TYPE_AABBOX3:
		{
			SetAABBox3(var.AsAABBox3());
		}
		break;
	case TYPE_FILEPATH:
		{
			SetFilePath(var.AsFilePath());
		}
		break;

	default:
		{
			//DVASSERT(0 && "Something went wrong with VariantType");
		}
	}
}
    
bool VariantType::AsBool() const
{
	DVASSERT(type == TYPE_BOOLEAN);
	return boolValue;
}

int32  VariantType::AsInt32() const
{
	DVASSERT(type == TYPE_INT32);
	return int32Value;
}
    
uint32  VariantType::AsUInt32() const
{
    DVASSERT(type == TYPE_UINT32);
    return uint32Value;
}

float32  VariantType::AsFloat() const
{
	DVASSERT(type == TYPE_FLOAT);
	return floatValue;
}

float64 VariantType::AsFloat64() const
{
    DVASSERT(type == TYPE_FLOAT64);
    return float64Value;
}

const String &  VariantType::AsString() const
{
	DVASSERT(type == TYPE_STRING);
	return *stringValue;
}

const WideString & VariantType::AsWideString() const
{
	DVASSERT(type == TYPE_WIDE_STRING);
	return *wideStringValue;
}
	
const uint8 *VariantType::AsByteArray() const
{
	DVASSERT(type == TYPE_BYTE_ARRAY);
    const auto vec = static_cast<const Vector<uint8>*>(pointerValue);
    return vec->empty() ? nullptr : vec->data();
}
	
int32 VariantType::AsByteArraySize() const
{
	DVASSERT(type == TYPE_BYTE_ARRAY);
    const auto vec = static_cast<const Vector<uint8>*>(pointerValue);
    return static_cast<int32>(vec->size());
}

KeyedArchive *VariantType::AsKeyedArchive() const
{
    DVASSERT(type == TYPE_KEYED_ARCHIVE);
    return (KeyedArchive*)pointerValue;
}
    
int64 VariantType::AsInt64() const
{
    DVASSERT(type == TYPE_INT64);
    return int64Value;
}

uint64 VariantType::AsUInt64() const
{
    DVASSERT(type == TYPE_UINT64);
    return uint64Value;
}
    
const Vector2 & VariantType::AsVector2() const
{
    DVASSERT(type == TYPE_VECTOR2);
    return *vector2Value;
}

const Vector3 & VariantType::AsVector3() const
{
    DVASSERT(type == TYPE_VECTOR3);
    return *vector3Value;
}

const Vector4 & VariantType::AsVector4() const
{
    DVASSERT(type == TYPE_VECTOR4);
    return *vector4Value;
}

const Matrix2 & VariantType::AsMatrix2() const
{
    DVASSERT(type == TYPE_MATRIX2);
    return *matrix2Value;
}
    
const Matrix3 & VariantType::AsMatrix3() const
{
    DVASSERT(type == TYPE_MATRIX3);
    return *matrix3Value;
}
    
const Matrix4 & VariantType::AsMatrix4() const
{
    DVASSERT(type == TYPE_MATRIX4);
    return *matrix4Value;
}

//const void* const VariantType::AsPointer() const
//{
//	DVASSERT(type == TYPE_POINTER);
//	return (void *) *x64PointerValue;
//}
    
const Color & VariantType::AsColor() const
{
    DVASSERT(type == TYPE_COLOR);
    return *colorValue;
}

const FastName & VariantType::AsFastName() const
{
    DVASSERT(type == TYPE_FASTNAME);
    return *fastnameValue;
}

const AABBox3 & VariantType::AsAABBox3() const
{
	DVASSERT(type == TYPE_AABBOX3);
	return *aabbox3;
}

const FilePath & VariantType::AsFilePath() const
{
	DVASSERT(type == TYPE_FILEPATH);
	return *filepathValue;
}


bool VariantType::Write(File * fp) const
{
	DVASSERT(type != TYPE_NONE)
	int32 written = fp->Write(&type, 1);
	if (written != 1)
    {
        return false;
    }
	
	switch(type)
	{
	case TYPE_BOOLEAN:
		{
			written = fp->Write(&boolValue, 1);
            if (written != 1)
            {
                return false;
            }
        }
		break;
	case TYPE_INT32:
		{
			written = fp->Write(&int32Value, 4);
            if (written != 4)
            {
                return false;
            }
        }
		break;	
    case TYPE_UINT32:
    {
        written = fp->Write(&uint32Value, 4);
        if (written != 4)
        {
            return false;
        }
    }
        break;	
	case TYPE_FLOAT:
		{
			written = fp->Write(&floatValue, 4);
            if (written != 4)
            {
                return false;
            }
		}
		break;	
    case TYPE_FLOAT64:
        {
            written = fp->Write(&float64Value, sizeof(float64));
            if (written != sizeof(float64))
            {
                return false;
            }
        }
        break;	
	case TYPE_STRING:
		{
			int32 len = (int32)stringValue->length();
			written = fp->Write(&len, 4);
            if (written != 4)
            {
                return false;
            }
			
			written = fp->Write(stringValue->c_str(), len);
            if (written != len)
            {
                return false;
            }
		}
		break;	
	case TYPE_WIDE_STRING:
		{
			int32 len = (int32)wideStringValue->length();
			written = fp->Write(&len, 4);
            if (written != 4)
            {
                return false;
            }
			
			written = fp->Write(wideStringValue->c_str(), len * sizeof(wchar_t));
            if (written != len * (int)sizeof(wchar_t))
            {
                return false;
            }
		}
		break;
	case TYPE_BYTE_ARRAY:
		{
			int32 len = (int32)((Vector<uint8>*)pointerValue)->size();
			written = fp->Write(&len, 4);
            if (written != 4)
            {
                return false;
            }
            if (0 != len)
            {
                written = fp->Write(&((Vector<uint8>*)pointerValue)->front(), len);
                if (written != len)
                {
                    return false;
                }
            }
		}
		break;	
    case TYPE_KEYED_ARCHIVE:
		{
            DynamicMemoryFile *pF = DynamicMemoryFile::Create(File::WRITE|File::APPEND);
            ((KeyedArchive *)pointerValue)->Save(pF);
			int32 len = pF->GetSize();
			written = fp->Write(&len, 4);
            if (written != 4)
            {
                return false;
            }
			
			written = fp->Write(pF->GetData(), len);
            SafeRelease(pF);
            if (written != len)
            {
                return false;
            }
		}
        break;
    case TYPE_INT64:
		{
            written = fp->Write(&int64Value, sizeof(int64));
            if (written != sizeof(int64))
            {
                return false;
            }
		}
            break;
    case TYPE_UINT64:
		{
            written = fp->Write(&uint64Value, sizeof(uint64));
            if (written != sizeof(uint64))
            {
                return false;
            }
		}
            break;
    case TYPE_VECTOR2:
		{
            written = fp->Write(vector2Value, sizeof(Vector2));
            if (written != sizeof(Vector2))
            {
                return false;
            }
		}
            break;
    case TYPE_VECTOR3:
		{
            written = fp->Write(vector3Value, sizeof(Vector3));
            if (written != sizeof(Vector3))
            {
                return false;
            }
		}
            break;
    case TYPE_VECTOR4:
		{
            written = fp->Write(vector4Value, sizeof(Vector4));
            if (written != sizeof(Vector4))
            {
                return false;
            }
		}
            break;
    case TYPE_MATRIX2:
		{
            written = fp->Write(matrix2Value, sizeof(Matrix2));
            if (written != sizeof(Matrix2))
            {
                return false;
            }
		}
            break;
    case TYPE_MATRIX3:
		{
            written = fp->Write(matrix3Value, sizeof(Matrix3));
            if (written != sizeof(Matrix3))
            {
                return false;
            }
		}
            break;
    case TYPE_MATRIX4:
		{
            written = fp->Write(matrix4Value, sizeof(Matrix4));
            if (written != sizeof(Matrix4))
            {
                return false;
            }
		}
            break;
    case TYPE_COLOR:
		{
            uint32 size = static_cast<uint32>(sizeof(Color));
            written = fp->Write(colorValue->color, size);
            if (written != size)
            {
                return false;
            }
		}
        break;
    case TYPE_FASTNAME:
	    {
			int32 len = static_cast<int32>(strlen(fastnameValue->c_str()));
			written = fp->Write(&len, 4);
            if (written != 4)
            {
                return false;
            }
        
			written = fp->Write(fastnameValue->c_str(), len);
            if (written != len)
            {
                return false;
            }
	    }
        break;
	case TYPE_AABBOX3:
		{
			written = fp->Write(aabbox3, sizeof(AABBox3));
			if (written != sizeof(AABBox3))
            {
                return false;
            }
		}
		break;
	case TYPE_FILEPATH:
		{
			String str = filepathValue->GetAbsolutePathname();
			int32 len = (int32) str.length();
			written = fp->Write(&len, 4);
            if (written != 4)
            {
                return false;
            }

			written = fp->Write(str.c_str(), len);
            if (written != len)
            {
                return false;
            }
		}
		break;	
            
	}
	return true;
}

bool VariantType::Read(File * fp)
{
	int32 read = fp->Read(&type, 1);
    if (read == 0)
    {
        return false;
    }

	ReleasePointer();
	switch(type)
	{
		case TYPE_BOOLEAN:
		{
			read = fp->Read(&boolValue, 1);
            if (read != 1)
            {
                return false;
            }
		}
		break;
		case TYPE_INT32:
		{
			read = fp->Read(&int32Value, 4);
            if (read != 4)
            {
                return false;
            }
		}
        break;	
		case TYPE_UINT32:
		{
			read = fp->Read(&uint32Value, 4);
            if (read != 4)
            {
                return false;
            }
		}
        break;	
		case TYPE_FLOAT:
		{
			read = fp->Read(&floatValue, 4);
            if (read != 4)
            {
                return false;
            }
		}
        break;	
        case TYPE_FLOAT64:
        {
            read = fp->Read(&float64Value, sizeof(float64));
            if (read != sizeof(float64))
            {
                return false;
            }
        }
            break;	
		case TYPE_STRING:
		{
			int32 len;
			read = fp->Read(&len, 4);
            if (read != 4)
            {
                return false;
            }
			
			char *buf = new char[len + 1];
			read = fp->Read(buf, len);
			buf[len] = 0;
			stringValue = new String(buf);
			delete [] buf;
            if (read != len)
            {
                return false;
            }
			
		}
        break;	
		case TYPE_WIDE_STRING:
		{ 
			int32 len;
			read = fp->Read(&len, 4);
            if (read != 4)
            {
                return false;
            }
			
            wideStringValue = new WideString();
			wideStringValue->resize(len);
			for (int k = 0; k < len; ++k)
			{
				wchar_t c;
				read = fp->Read(&c, sizeof(wchar_t));
                if (read != sizeof(wchar_t))
                {
                    return false;
                }
				(*wideStringValue)[k] = c;
			}
		}
        break;
		case TYPE_BYTE_ARRAY:
		{
			int32 len;
			read = fp->Read(&len, 4);
            if (read != 4)
            {
                return false;
            }
            
            pointerValue = static_cast<void*>(new Vector<uint8>(len));
            if (0 != len)
            {
                read = fp->Read(&((Vector<uint8>*)pointerValue)->front(), len);
                if (read != len)
                {
                    return false;
                }
            }
		}
        break;	
		case TYPE_KEYED_ARCHIVE:
		{
			int32 len;
			read = fp->Read(&len, 4);
            if (read != 4)
            {
                return false;
            }
			
            uint8 *pData = new uint8[len];
			read = fp->Read(pData, len);
            if (read != len)
            {
                return false;
            }
            DynamicMemoryFile *pF = DynamicMemoryFile::Create(pData, len, File::READ);
            pointerValue = new KeyedArchive();
            ((KeyedArchive*)pointerValue)->Load(pF);
            SafeRelease(pF);
            SafeDeleteArray(pData);
		}
        break;
        case TYPE_INT64:
		{
            read = fp->Read(&int64Value, sizeof(int64));
            if (read != sizeof(int64))
            {
                return false;
            }
		}
            break;
        case TYPE_UINT64:
		{
            read = fp->Read(&uint64Value, sizeof(uint64));
            if (read != sizeof(uint64))
            {
                return false;
            }
		}
            break;
        case TYPE_VECTOR2:
		{
            vector2Value = new Vector2;
            read = fp->Read(vector2Value, sizeof(Vector2));
            if (read != sizeof(Vector2))
            {
                return false;
            }
		}
            break;
        case TYPE_VECTOR3:
		{
            vector3Value = new Vector3;
            read = fp->Read(vector3Value, sizeof(Vector3));
            if (read != sizeof(Vector3))
            {
                return false;
            }
		}
            break;
        case TYPE_VECTOR4:
		{
            vector4Value = new Vector4;
            read = fp->Read(vector4Value, sizeof(Vector4));
            if (read != sizeof(Vector4))
            {
                return false;
            }
		}
            break;
        case TYPE_MATRIX2:
		{
            matrix2Value = new Matrix2;
            read = fp->Read(matrix2Value, sizeof(Matrix2));
            if (read != sizeof(Matrix2))
            {
                return false;
            }
		}
            break;
        case TYPE_MATRIX3:
		{
            matrix3Value = new Matrix3;
            read = fp->Read(matrix3Value, sizeof(Matrix3));
            if (read != sizeof(Matrix3))
            {
                return false;
            }
		}
            break;
        case TYPE_MATRIX4:
		{
            matrix4Value = new Matrix4;
            read = fp->Read(matrix4Value, sizeof(Matrix4));
            if (read != sizeof(Matrix4))
            {
                return false;
            }
		}
            break;
        case TYPE_COLOR:
			{
				colorValue = new Color;
                read = fp->Read(colorValue->color, sizeof(Color));
                if (read != sizeof(float32) * 4)
                {
                    return false;
                }
			}
            break;

        case TYPE_FASTNAME:
			{
				int32 len = 0;
				read = fp->Read(&len, 4);
                if (read != 4)
                {
                    return false;
                }
			
				char *buf = new char[len + 1];
				read = fp->Read(buf, len);
				buf[len] = 0;
				fastnameValue = new FastName(buf);
				delete [] buf;
                if (read != len)
                {
                    return false;
                }
			}
            break;

		case TYPE_AABBOX3:
			{
				aabbox3 = new AABBox3;
				read = fp->Read(aabbox3, sizeof(AABBox3));
                if (read != sizeof(AABBox3))
                {
                    return false;
                }
			}
			break;
		case TYPE_FILEPATH:
			{
				int32 len;
				read = fp->Read(&len, 4);
                if (read != 4)
                {
                    return false;
                }

				char *buf = new char[len + 1];
				read = fp->Read(buf, len);
				buf[len] = 0;
				filepathValue = new FilePath(buf);
				delete [] buf;
                if (read != len)
                {
                    return false;
                }
			}
			break;	
		default:
		{
            return false;
		}
	}
	return true;
}
	
	
void VariantType::ReleasePointer()
{
    if (pointerValue) 
    {
        switch (type) 
        {
            case TYPE_BYTE_ARRAY:
            {
                delete (Vector<uint8> *)pointerValue;
            }
                break;
            case TYPE_KEYED_ARCHIVE:
            {
                ((KeyedArchive *)pointerValue)->Release();
            }
                break;
            case TYPE_VECTOR2:
            {
                delete vector2Value;
            }
                break;
            case TYPE_VECTOR3:
            {
                delete vector3Value;
            }
                break;
            case TYPE_VECTOR4:
            {
                delete vector4Value;
            }
                break;
            case TYPE_MATRIX2:
            {
                delete matrix2Value;
            }
                break;
            case TYPE_MATRIX3:
            {
                delete matrix3Value;
            }
                break;
            case TYPE_MATRIX4:
            {
                delete matrix4Value;
            }
                break;

            case TYPE_STRING:
            {
                delete stringValue;
            }
                break;

            case TYPE_WIDE_STRING:
            {
                delete wideStringValue;
            }
                break;
            case TYPE_COLOR:
            {
                delete colorValue;
            }
                break;
            case TYPE_FASTNAME:
            {
                delete fastnameValue;
            }
            break;
            case TYPE_AABBOX3:
            {
                delete aabbox3;
            }
            break;
            case TYPE_FILEPATH:
            {
                delete filepathValue;
            }
            break;
        }
        
        // It is enough to set only pointerValue to nullptr - all other pointers are in union, so
        // actually points to the same address and thus will be set to nullptr too.
        pointerValue = nullptr;
    }
}

bool VariantType::operator==(const VariantType& other) const
{
    bool isEqual = false;
    if(type == other.type)
    {
        switch(type)
        {
            case TYPE_BOOLEAN:
                isEqual = (AsBool() == other.AsBool());
                break;
		    case TYPE_INT32:
                isEqual = (AsInt32() == other.AsInt32());
                break;
		    case TYPE_FLOAT:
                isEqual = (AsFloat() == other.AsFloat());
                break;
            case TYPE_FLOAT64:
                isEqual = (AsFloat64() == other.AsFloat64());
                break;
		    case TYPE_STRING:
                isEqual = (AsString() == other.AsString());
                break;
            case TYPE_WIDE_STRING:
                isEqual = (AsWideString() == other.AsWideString());
                break;
		    case TYPE_BYTE_ARRAY:
            {
                int32 byteArraySize = AsByteArraySize();
                if(byteArraySize == other.AsByteArraySize())
                {
                    isEqual = true;
                    const uint8* byteArray = AsByteArray();
                    const uint8* otherByteArray = other.AsByteArray();
                    if(byteArray != otherByteArray)
                    {
                        for(int32 i = 0; i < byteArraySize; ++i)
                        {
                            if(byteArray[i]!=otherByteArray[i])
                            {
                                isEqual = false;
                                break;
                            }
                        }
                    }
                }
            }
                break;
            case TYPE_UINT32:
                isEqual = (AsUInt32() == other.AsUInt32());
                break;
            case TYPE_KEYED_ARCHIVE:
                {
                    KeyedArchive* keyedArchive = AsKeyedArchive();
                    KeyedArchive* otherKeyedArchive = other.AsKeyedArchive();
                    if(keyedArchive && otherKeyedArchive)
                    {
                        isEqual = true;
                        if(keyedArchive != otherKeyedArchive)
                        {                                
                            const Map<String, VariantType*> &data = keyedArchive->GetArchieveData();
                            const Map<String, VariantType*> &otherData = otherKeyedArchive->GetArchieveData();
                            for(const auto &obj : data)
                            {
                                Map<String, VariantType*>::const_iterator findIt = otherData.find(obj.first);
                                if(findIt != otherData.end())
                                {
                                    if(obj.second != findIt->second)
                                    {
                                        if((*obj.second) != (*findIt->second))
                                        {
                                            isEqual = false;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    isEqual = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            case TYPE_INT64:
                isEqual = (AsInt64() == other.AsInt64());
                break;
            case TYPE_UINT64:
                isEqual = (AsUInt64()== other.AsUInt64());
                break;
            case TYPE_VECTOR2:
            {
                isEqual = ( AsVector2() == other.AsVector2());
            }
                break;
            case TYPE_VECTOR3:
            {
                isEqual = ( AsVector3() == other.AsVector3());
            }
                break;
            case TYPE_VECTOR4:
            {
                isEqual = ( AsVector4() == other.AsVector4());
            }
                break;
            case TYPE_MATRIX2:
            {
                isEqual = ( AsMatrix2() == other.AsMatrix2());
            }
                break;
            case TYPE_MATRIX3:
            {
                isEqual = ( AsMatrix3() == other.AsMatrix3());
            }
                break;
            case TYPE_MATRIX4:
            {
                isEqual = ( AsMatrix4() == other.AsMatrix4());
            }
                break;
			case  TYPE_COLOR:
            {
                isEqual = (AsColor() == other.AsColor());
            }
				break;
			case  TYPE_FASTNAME:
            {
                isEqual = (AsFastName() == other.AsFastName());
            }
				break;
			case  TYPE_AABBOX3:
				{
					isEqual = (AsAABBox3() == other.AsAABBox3());
				}
				break;
			case TYPE_FILEPATH:
				{
					isEqual = (AsFilePath() == other.AsFilePath());
				}
				break;
                
        }
    }
    return isEqual;
}

bool VariantType::operator!=(const VariantType& other) const
{
    return (!(*this == other));
}

VariantType& VariantType::operator=(const VariantType& other)
{
	SetVariant(other);
	return *this;
}

const MetaInfo* VariantType::Meta()
{
	if(type >= 0 && type < TYPES_COUNT)
	{
		return variantNamesMap[type].variantMeta;
	}

	return nullptr;
}

void* VariantType::MetaObject()
{
	const void *ret = nullptr;

	switch(type)
	{
	case TYPE_BOOLEAN:
		ret = &boolValue;
		break;
	case TYPE_INT32:
		ret = &int32Value;
		break;	
	case TYPE_UINT32:
		ret = &uint32Value;
		break;	
    case TYPE_INT64:
        ret = &int64Value;
        break;
    case TYPE_UINT64:
        ret = &uint64Value;
        break;
	case TYPE_FLOAT:
		ret = &floatValue;
		break;	
    case TYPE_FLOAT64:
        ret = &float64Value;
        break;	
	case TYPE_STRING:
		ret = stringValue;
		break;	
	case TYPE_WIDE_STRING:
		ret = wideStringValue;
		break;
	case TYPE_VECTOR2:
	case TYPE_BYTE_ARRAY:
	case TYPE_VECTOR3:
	case TYPE_VECTOR4:
	case TYPE_MATRIX2:
	case TYPE_MATRIX3:
	case TYPE_MATRIX4:
	case TYPE_COLOR:
	case TYPE_FASTNAME:
	case TYPE_AABBOX3:
	case TYPE_FILEPATH:
		ret = pointerValue;
		break;
	case TYPE_KEYED_ARCHIVE:
		ret = &pointerValue;
		break;
	default:
		{
			//DVASSERT(0 && "Something went wrong with VariantType");
		}
	}

	return (void *) ret;
}

VariantType VariantType::LoadData(const void *src, const MetaInfo *meta)
{
	VariantType v;
	uint8 type = TYPE_NONE;

	DVASSERT(nullptr != meta);

	for(int i = 0; i < TYPES_COUNT; ++i)
	{
		if(variantNamesMap[i].variantMeta == meta)
		{
			type = variantNamesMap[i].variantType;
			break;
		}
	}

	DVASSERT(nullptr != src);

	switch(type)
	{
	case TYPE_BOOLEAN:
		v.SetBool(*((bool *) src));
		break;
	case TYPE_INT32:
		v.SetInt32(*((int32 *) src));
		break;
	case TYPE_FLOAT:
		v.SetFloat(*((float *) src));
		break;
    case TYPE_FLOAT64:
        v.SetFloat64(*((float64 *)src));
        break;
	case TYPE_STRING:
		v.SetString(*((DAVA::String *) src));
		break;
	case TYPE_WIDE_STRING:
		v.SetWideString(*((DAVA::WideString *) src));
		break;
	case TYPE_UINT32:
		v.SetUInt32(*((uint32 *) src));
		break;
	//case TYPE_BYTE_ARRAY:
	//	break;
	case TYPE_KEYED_ARCHIVE:
		v.SetKeyedArchive(*((DAVA::KeyedArchive **) src));
		break;
	case TYPE_INT64:
		v.SetInt64(*((DAVA::int64 *) src));
		break;
	case TYPE_UINT64:
		v.SetUInt64(*((DAVA::uint64 *) src));
		break;
	case TYPE_VECTOR2:
		v.SetVector2(*((DAVA::Vector2 *) src));
		break;
	case TYPE_VECTOR3:
		v.SetVector3(*((DAVA::Vector3 *) src));
		break;
	case TYPE_VECTOR4:
		v.SetVector4(*((DAVA::Vector4 *) src));
		break;
	case TYPE_MATRIX2:
		v.SetMatrix2(*((DAVA::Matrix2 *) src));
		break;
	case TYPE_MATRIX3:
		v.SetMatrix3(*((DAVA::Matrix3 *) src));
		break;
	case TYPE_MATRIX4:
		v.SetMatrix4(*((DAVA::Matrix4 *) src));
		break;
    case TYPE_COLOR:
        v.SetColor(*((DAVA::Color *) src));
        break;
    case TYPE_FASTNAME:
        v.SetFastName(*((DAVA::FastName *) src));
        break;
	case TYPE_AABBOX3:
		v.SetAABBox3(*((DAVA::AABBox3 *) src));
		break;
	case TYPE_FILEPATH:
		v.SetFilePath(*((DAVA::FilePath *) src));
		break;
	default:
		if(meta == MetaInfo::Instance<int8>())
		{
			v.SetInt32(*((DAVA::int8 *) src));
		}
		else if(meta == MetaInfo::Instance<uint8>())
		{
			v.SetUInt32(*((DAVA::uint8 *) src));
		}
		else
		{
			printf("MetaType: %s, size %d, is pointer %d, introspection %p\n", meta->GetTypeName(), meta->GetSize(), meta->IsPointer(), meta->GetIntrospection());
			if(nullptr != meta->GetIntrospection())
			{
				printf("Introspection: %s\n", meta->GetIntrospection()->Name().c_str());
			}
			DVASSERT(0 && "Don't know how to load data for such VariantType");
		}
	}

	return v;
}

void VariantType::SaveData(void *dst, const MetaInfo *meta, const VariantType &val)
{
	MetaInfo *valMeta = nullptr;

	DVASSERT(nullptr != meta);

	for(int i = 0; i < TYPES_COUNT; ++i)
	{
		if(variantNamesMap[i].variantType == val.type)
		{
			valMeta = variantNamesMap[i].variantMeta;
			break;
		}
	}

	DVASSERT(nullptr != valMeta)

	// Destination meta type differ from source meta type
	// this happen only for int8 and uint8 types, because we are storing them in int32 and uint32
	if(meta != valMeta)
	{
		if(meta == MetaInfo::Instance<int8>() && valMeta == MetaInfo::Instance<int32>())
		{
			*((DAVA::int8 *) dst) = (int8) val.AsInt32();
		}
		else if(meta == MetaInfo::Instance<uint8>() && valMeta == MetaInfo::Instance<int32>())
		{
			*((DAVA::uint8 *) dst) = (uint8) val.AsUInt32();
		}
		else
		{
			DVASSERT(0 && "Destination type differ from source type");
		}
	}
	else
	{
		switch(val.type)
		{
		case TYPE_BOOLEAN:
			*((bool *) dst) = val.AsBool();
			break;
		case TYPE_INT32:
			*((int32 *) dst) = val.AsInt32();
			break;
		case TYPE_FLOAT:
			*((float *) dst) = val.AsFloat();
			break;
        case TYPE_FLOAT64:
            *((float64 *)dst) = val.AsFloat64();
            break;
		case TYPE_STRING:
			*((DAVA::String *) dst) = val.AsString();
			break;
		case TYPE_WIDE_STRING:
			*((DAVA::WideString *) dst) = val.AsWideString();
			break;
		case TYPE_UINT32:
			*((uint32 *) dst) = val.AsUInt32();
			break;
			//case TYPE_BYTE_ARRAY:
			//	break;
		case TYPE_KEYED_ARCHIVE:
			{
				DAVA::KeyedArchive *dstArchive = *((DAVA::KeyedArchive **) dst);
				if(nullptr != dstArchive)
				{
					dstArchive->DeleteAllKeys();
                    for(const auto &obj : val.AsKeyedArchive()->GetArchieveData())
                    {
                        dstArchive->SetVariant(obj.first, *obj.second);
                    }
				}
			}
			break;
		case TYPE_INT64:
			*((DAVA::int64 *) dst) = val.AsInt64();
			break;
		case TYPE_UINT64:
			*((DAVA::uint64 *) dst) = val.AsUInt64();
			break;
		case TYPE_VECTOR2:
			*((DAVA::Vector2 *) dst) = val.AsVector2();
			break;
		case TYPE_VECTOR3:
			*((DAVA::Vector3 *) dst) = val.AsVector3();
			break;
		case TYPE_VECTOR4:
			*((DAVA::Vector4 *) dst) = val.AsVector4();
			break;
		case TYPE_MATRIX2:
			*((DAVA::Matrix2 *) dst) = val.AsMatrix2();
			break;
		case TYPE_MATRIX3:
			*((DAVA::Matrix3 *) dst) = val.AsMatrix3();
			break;
		case TYPE_MATRIX4:
			*((DAVA::Matrix4 *) dst) = val.AsMatrix4();
			break;
		case TYPE_COLOR:
			*((DAVA::Color *) dst) = val.AsColor();
			break;
		case TYPE_FASTNAME:
			*((DAVA::FastName *) dst) = val.AsFastName();
			break;
		case TYPE_AABBOX3:
			*((DAVA::AABBox3 *) dst) = val.AsAABBox3();
			break;
		case TYPE_FILEPATH:
			*((DAVA::FilePath *) dst) = val.AsFilePath();
			break;
		default:
			DVASSERT(0 && "Don't know how to save data from such VariantType");
			break;
		}
	}
}

VariantType VariantType::FromType(int type)
{
	VariantType v;

	bool found = false;
	for(int i = 0; i < TYPES_COUNT; ++i)
	{
		if(variantNamesMap[i].variantType == type)
		{
			found = true;
			break;
		}
	}

	DVASSERT(true == found);

	switch(type)
	{
	case TYPE_BOOLEAN:
		v.SetBool(false);
		break;
	case TYPE_INT32:
		v.SetInt32(0);
		break;
	case TYPE_FLOAT:
		v.SetFloat(0.0);
		break;
    case TYPE_FLOAT64:
        v.SetFloat64(0.0);
        break;
	case TYPE_STRING:
		v.SetString("");
		break;
	case TYPE_WIDE_STRING:
		v.SetWideString(WideString());
		break;
	case TYPE_UINT32:
		v.SetUInt32(0);
		break;
	case TYPE_BYTE_ARRAY:
		v.SetByteArray(nullptr, 0);
		break;
	case TYPE_KEYED_ARCHIVE:
		{
			KeyedArchive *ka = new KeyedArchive();
			v.SetKeyedArchive(ka);
			ka->Release();
		}
		break;
	case TYPE_INT64:
		v.SetInt64(0);
		break;
	case TYPE_UINT64:
		v.SetUInt64(0);
		break;
	case TYPE_VECTOR2:
		v.SetVector2(Vector2());
		break;
	case TYPE_VECTOR3:
		v.SetVector3(Vector3());
		break;
	case TYPE_VECTOR4:
		v.SetVector4(Vector4());
		break;
	case TYPE_MATRIX2:
		v.SetMatrix2(Matrix2());
		break;
	case TYPE_MATRIX3:
		v.SetMatrix3(Matrix3());
		break;
	case TYPE_MATRIX4:
		v.SetMatrix4(Matrix4());
		break;
	case TYPE_COLOR:
		v.SetColor(Color());
		break;
	case TYPE_FASTNAME:
		v.SetFastName(FastName(""));
		break;
	case TYPE_AABBOX3:
		v.SetAABBox3(AABBox3());
		break;
	case TYPE_FILEPATH:
		v.SetFilePath(FilePath());
		break;
	default:
		DVASSERT(0 && "Don't know how to create such VariantType");
	}

	return v;
}

VariantType VariantType::Convert(const VariantType &val, int type)
{
	VariantType ret;

	if(val.type == type)
	{
		ret = val;
	}
	else
	{
		switch (type)
		{
		case TYPE_INT32:
			{
				switch (val.type)
				{
				case TYPE_UINT32: ret.SetInt32((DAVA::int32) val.AsUInt32()); break;
				case TYPE_UINT64: ret.SetInt32((DAVA::int32) val.AsUInt64()); break;
				case TYPE_INT64: ret.SetInt32((DAVA::int32) val.AsUInt64()); break;
				default: break;
				}
			}
			break;

		case TYPE_UINT32:
			{
				switch (val.type)
				{
				case TYPE_INT32: ret.SetUInt32((DAVA::uint32) val.AsInt32()); break;
				case TYPE_UINT64: ret.SetUInt32((DAVA::uint32) val.AsUInt64()); break;
				case TYPE_INT64: ret.SetUInt32((DAVA::uint32) val.AsUInt64()); break;
				default: break;
				}
			}
			break;

		case TYPE_INT64:
			{
				switch (val.type)
				{
				case TYPE_UINT32: ret.SetInt64((DAVA::int64) val.AsUInt32()); break;
				case TYPE_UINT64: ret.SetInt64((DAVA::int64) val.AsUInt64()); break;
				case TYPE_INT32: ret.SetInt64((DAVA::int64) val.AsInt32()); break;
				default: break;
				}
			}
			break;

		case TYPE_UINT64:
			{
				switch (val.type)
				{
				case TYPE_UINT32: ret.SetInt64((DAVA::uint64) val.AsUInt32()); break;
				case TYPE_INT64: ret.SetInt64((DAVA::uint64) val.AsInt64()); break;
				case TYPE_INT32: ret.SetInt64((DAVA::uint64) val.AsInt32()); break;
				default: break;
				}
			}
			break;

		default:
			break;
		}
	}

	return ret;
}

};