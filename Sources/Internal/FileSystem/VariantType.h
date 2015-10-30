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


#ifndef __DAVAENGINE_VARIANTTYPE_H__
#define __DAVAENGINE_VARIANTTYPE_H__

#include "Base/BaseTypes.h"

namespace DAVA 
{
    
class File;
	
struct MetaInfo;

class Vector2;
class Vector3;
class Vector4;
    
struct Matrix2;
struct Matrix3;
struct Matrix4;

class Color;
class KeyedArchive;
    
class FastName;
class AABBox3;
class FilePath;
    
/**
 \ingroup filesystem
 \brief Class to store value of all basic types in one instance. Can be used for various serialization / deserialization purposes.
 */
class VariantType
{
public:

    static const String TYPENAME_UNKNOWN;   // "unknown";
    static const String TYPENAME_BOOLEAN;   // "bool";
    static const String TYPENAME_INT32;     // "int32"
    static const String TYPENAME_UINT32;    // "uint32"
    static const String TYPENAME_INT64;     // "int64"
    static const String TYPENAME_UINT64;    // "uint64"

    static const String TYPENAME_FLOAT;     // "float"
    static const String TYPENAME_FLOAT64; // "float64"
    static const String TYPENAME_STRING;    // "string"
    static const String TYPENAME_WIDESTRING;// "wideString"
    static const String TYPENAME_BYTE_ARRAY;// "byteArray"
    static const String TYPENAME_KEYED_ARCHIVE;// "keyedArchive"
    static const String TYPENAME_VECTOR2;   // "Vector2"
    static const String TYPENAME_VECTOR3;   // "Vector3"
    static const String TYPENAME_VECTOR4;   // "Vector4"
    static const String TYPENAME_MATRIX2;   // "Matrix2"
    static const String TYPENAME_MATRIX3;   //  "Matrix3"
    static const String TYPENAME_MATRIX4;   // "Matrix4"

	static const String TYPENAME_COLOR;     // "Color"
	static const String TYPENAME_FASTNAME;     // "FastName"
	static const String TYPENAME_AABBOX3;     // "AABBox3"
	static const String TYPENAME_FILEPATH;  // "FilePath"

    VariantType();
    VariantType(const VariantType& value);
    explicit VariantType(bool value);
    explicit VariantType(int32 value);
    explicit VariantType(uint32 value);
    explicit VariantType(float32 value);
    explicit VariantType(float64 value);
    explicit VariantType(const String& value);
    explicit VariantType(const WideString& value);
    explicit VariantType(const uint8* array, int32 arraySizeInBytes);
    explicit VariantType(KeyedArchive* archive);
    explicit VariantType(const int64& value);
    explicit VariantType(const uint64& value);
    explicit VariantType(const Vector2& value);
    explicit VariantType(const Vector3& value);
    explicit VariantType(const Vector4& value);
    explicit VariantType(const Matrix2 & value);
	explicit VariantType(const Matrix3 & value);
	explicit VariantType(const Matrix4 & value);
	explicit VariantType(const Color & value);
	explicit VariantType(const FastName & value);
	explicit VariantType(const AABBox3 & value);
	explicit VariantType(const FilePath & value);

	~VariantType();

    enum eVariantType
    {
        TYPE_NONE = 0,
        TYPE_BOOLEAN,
        TYPE_INT32,
        TYPE_FLOAT,
        TYPE_STRING,
        TYPE_WIDE_STRING,
        TYPE_BYTE_ARRAY,
        TYPE_UINT32,
        TYPE_KEYED_ARCHIVE,
        TYPE_INT64,
        TYPE_UINT64,
        TYPE_VECTOR2,
        TYPE_VECTOR3,
        TYPE_VECTOR4,
        TYPE_MATRIX2,
        TYPE_MATRIX3,
        TYPE_MATRIX4,
        TYPE_COLOR,
        TYPE_FASTNAME,
        TYPE_AABBOX3,
        TYPE_FILEPATH,
        TYPE_FLOAT64,

        TYPES_COUNT // every new type should be always added to the end for compatibility with old archives
    };
    uint8 type;

    union {
        bool boolValue;
        int32 int32Value;
        uint32 uint32Value;
        float32 floatValue;
        float64 float64Value;

        int64 int64Value;
        uint64 uint64Value;

        Vector2* vector2Value;
        Vector3* vector3Value;
        Vector4* vector4Value;
        
        Matrix2* matrix2Value;
        Matrix3* matrix3Value;
        Matrix4* matrix4Value;

		const void* pointerValue;
        
        String* stringValue;
        WideString* wideStringValue;
		FilePath* filepathValue;
        
        Color* colorValue;
        FastName *fastnameValue;

		AABBox3 *aabbox3;
	};

    struct PairTypeName
    {
        eVariantType variantType;
        String variantName;
		MetaInfo *variantMeta;
        
        PairTypeName(eVariantType type, String name, MetaInfo *meta)
        {
            variantType = type;
            variantName = name;
			variantMeta = meta;
        }
    };
    
    const static Array<PairTypeName, TYPES_COUNT> variantNamesMap;
	
	// Functions
    
    
    inline eVariantType GetType() const;
    const String &GetTypeName() const;
	
	/**
		\brief Function to set bool value to variant type variable
		\param[in] value	value to set
	 */
	void SetBool(bool value);

	/**
		\brief Function to set int value to variant type variable
		\param[in] value value to set
	 */
	void SetInt32(int32 value);
    
	/**
        \brief Function to set int value to variant type variable
        \param[in] value value to set
	 */
    void SetUInt32(uint32 value);

	/**
		\brief Function to set float value to variant type variable
		\param[in] value	value to set
	 */
	void SetFloat(float32 value);

    /**
         \brief Function to set float value to variant type variable
         \param[in] value	value to set
     */
    void SetFloat64(float64 value);

    /**
		\brief Function to set string value to variant type variable
		\param[in] value	value to set
	 */
    void SetString(const String& value);

    /**
		\brief Function to set wide string value to variant type variable
		\param[in] value	value to set
	 */
    void SetWideString(const WideString& value);

    /**
	 \brief Function to set byte array value to variant type variable
	 \param[in] value	value to set
	 \param[in] arraySizeInBytes	size of the array in bytes
	 */
    void SetByteArray(const uint8* array, int32 arraySizeInBytes);

    /**
	 \brief Function to set KeyedArchive to variation type variable.
     Archive is copying into the variable.
	 \param[in] archive	archive to set (Archive is retains inside variable type)
	 */
    void SetKeyedArchive(KeyedArchive* archive);

    /**
     \brief Function to set int64 value to variant type variable
     \param[in] value	value to set
	 */
	void SetInt64(const int64 & value);
    
    /**
     \brief Function to set uint64 value to variant type variable
     \param[in] value	value to set
	 */
	void SetUInt64(const uint64 & value);

    /**
     \brief Function to set Vector2 value to variant type variable
     \param[in] value	value to set
	 */
	void SetVector2(const Vector2 & value);

    /**
     \brief Function to set Vector3 value to variant type variable
     \param[in] value	value to set
	 */
	void SetVector3(const Vector3 & value);
    
    /**
     \brief Function to set Vector4 value to variant type variable
     \param[in] value	value to set
	 */
	void SetVector4(const Vector4 & value);
    
    /**
     \brief Function to set Matrix2 value to variant type variable
     \param[in] value	value to set
	 */
	void SetMatrix2(const Matrix2 & value);
    
    /**
     \brief Function to set Matrix3 value to variant type variable
     \param[in] value	value to set
	 */
	void SetMatrix3(const Matrix3 & value);
    
    /**
     \brief Function to set Matrix4 value to variant type variable
     \param[in] value	value to set
	 */
	void SetMatrix4(const Matrix4 & value);

	void SetVariant(const VariantType& value);

    /**
     \brief Function to set Color value to variant type variable
     \param[in] value	value to set
	 */
	void SetColor(const Color & value);

    /**
     \brief Function to set FastName value to variant type variable
     \param[in] value	value to set
	 */
	void SetFastName(const FastName & value);
    
    /**
		 \brief Function to set AABBox3 value to variant type variable
		 \param[in] value	value to set
	 */
	void SetAABBox3(const AABBox3 & value);

	/**
		\brief Function to set FilePath value to variant type variable
		\param[in] value	value to set
	 */
	void SetFilePath(const FilePath & value);

	/**
		\brief Function to return bool value from variable
		\returns value of variable, or generate assert if variable type is different
	 */
	bool AsBool() const;

	/**
		\brief Function to return int value from variable
		\returns value of variable, or generate assert if variable type is different
	 */
	int32 AsInt32() const;

	/**
        \brief Function to return int value from variable
        \returns value of variable, or generate assert if variable type is different
	 */
	uint32 AsUInt32() const;

	/**
		\brief Function to return float value from variable
		\returns value of variable, or generate assert if variable type is different
	 */
	float32 AsFloat() const;

    /**
         \brief Function to return float value from variable
         \returns value of variable, or generate assert if variable type is different
     */
    float64 AsFloat64() const;

    /**
		\brief Function to return string value from variable
		\returns value of variable, or generate assert if variable type is different
	 */
    const String& AsString() const;

    /**
		\brief Function to return wide string value from variable
		\returns value of variable, or generate assert if variable type is different
	 */
    const WideString& AsWideString() const;

    /**
	 \brief Function to return array from variable
	 \returns value of variable, or generate assert if variable type is different
	 */
    const uint8* AsByteArray() const;
    /**
	 \brief Function to return array size from variable
	 \returns array size in bytes variable, or generate assert if variable type is different
	 */
    int32 AsByteArraySize() const;

    /**
	 \brief Function to return keyed archive from variable. Returns pointer to the KeyedArchive inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     KeyedArchive *AsKeyedArchive() const;
      
    /**
	 \brief Function to return int value from variable.Returns pointer to the int64 inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     int64 AsInt64() const;

    /**
	 \brief Function to return unsigned int value from variable.Returns pointer to the uint64 inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     uint64 AsUInt64() const;
    
    /**
	 \brief Function to return vector2 from variable. Returns pointer to the vector2 inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     const Vector2 &AsVector2() const;
    
    /**
	 \brief Function to return vector3 from variable. Returns pointer to the vector3 inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     const Vector3 &AsVector3() const;
    
    /**
	 \brief Function to return vector4 from variable. Returns pointer to the vector4 inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     const Vector4 &AsVector4() const;
    
    /**
	 \brief Function to return matrix2 from variable. Returns pointer to the matrix2 inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     const Matrix2 &AsMatrix2() const;
    
    /**
	 \brief Function to return matrix3 from variable. Returns pointer to the matrix3 inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     const Matrix3 &AsMatrix3() const;
    
    /**
	 \brief Function to return matrix4 from variable. Returns pointer to the matrix4 inside.
	 \returns value of variable, or generate assert if variable type is different
	 */
     const Matrix4 &AsMatrix4() const;

    /**
         \brief Function to return Color from variable. Returns pointer to the Color inside.
         \returns value of variable, or generate assert if variable type is different
     */
    const Color &AsColor() const;

    /**
         \brief Function to return FastName from variable. Returns pointer to the FastName inside.
         \returns value of variable, or generate assert if variable type is different
     */
    const FastName &AsFastName() const;


    /**
         \brief Function to return AABBox3 from variable. Returns pointer to the FastName inside.
         \returns value of variable, or generate assert if variable type is different
     */
    const AABBox3 &AsAABBox3() const;

    /**
         \brief Function to return FilePath from variable.
		 \returns value of variable, or generate assert if variable type is different
     */
    const FilePath &AsFilePath() const;

	// File read & write helpers
	
	/**
		\brief Function to write variable to file, from it current position
		\returns true if variable written successfully
	 */
	bool Write(File * fp) const;
	
	/**
		\brief Function to read variable to file, to it current position
		\returns true if variable read successfully
	 */
	bool Read(File * fp);

    /**
		\brief Operator to compare variant types
		\returns true if values are equal
	 */
    bool operator==(const VariantType& other) const;

    /**
		\brief Operator to compare variant types
		\returns true if values are not equal
	 */
    bool operator!=(const VariantType& other) const;

	VariantType& operator=(const VariantType& other);

	const MetaInfo* Meta();
	void* MetaObject();

	static VariantType LoadData(const void *src, const MetaInfo *meta);
	static void SaveData(void *dst, const MetaInfo *meta, const VariantType &val);

	static VariantType FromType(int type);
	static VariantType Convert(const VariantType &val, int type);
    
private:
	// This constructor is private to prevent creation of VariantType from pointer
	// Without this, creating VariantType from any pointer will be automatically casted to BOOL 
	// by C++ compiler, that is completely wrong
	VariantType(void *);

	void ReleasePointer();
};
	
VariantType::eVariantType VariantType::GetType() const
{
    return (eVariantType)type;
}
	
};

#endif // __DAVAENGINE_VARIANTTYPE_H__