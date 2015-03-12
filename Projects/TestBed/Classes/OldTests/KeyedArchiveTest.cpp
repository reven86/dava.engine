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


#include "KeyedArchiveTest.h"
#include "Math/Math2D.h"


#define FILEPATHOLDFORMAT "~res:/KeyedArchives/oldTest.dat"
#define FILEPATHNEWFORMAT "~res:/KeyedArchives/newTest.dat"

#define BOOLMAPID       "bool[true]:"
#define BOOLVALUE       true

#define INT32MAPID      "int32[1]:"
#define INT32VALUE      1

#define UINT32MAPID     "uint32[2]:"
#define UINT32VALUE     2

#define FLOATMAPID      "float[3.5]:"
#define FLOATVALUE      3.5f

#define STRINGMAPID     "string[someString]:"
#define STRINGVALUE     "someString"

#define WSTRINGMAPID   "wideString[someWideString]:"
#define WSTRINGVALUE    L"someWideString"

#define BYTEARRMAPID    "byteArr[0x0A,0x0B,0x0C,0x0D]:"
#define BYTEARRVALUE    0x0A,0x0B,0x0C,0x0D
#define BYTEARRLENGTH   4

#define VARTYPEMAPID    "VarType[4]:"
#define VARTYPEVALUE    4

#define INT64MAPID      "int64[11]:"
#define INT64VALUE      11

#define UINT64MAPID     "uint64[12]:"
#define UINT64VALUE     12

#define VECTOR2MAPID    "vector2[5,5]:"
#define VECTOR2VALUE    5,5

#define VECTOR3MAPID    "vector3[6,6,6]:"
#define VECTOR3VALUE    6,6,6

#define VECTOR4MAPID    "vector4[7,7,7,7]:"
#define VECTOR4VALUE    7,7,7,7


#define MATRIX2MAPID    "matrix2[8,8,8,8]:"
#define MATRIX2VALUE    8,8,8,8

#define MATRIX3MAPID    "matrix3[9,9,9,9,9,9,9,9,9]:"
#define MATRIX3VALUE    9,9,9,9,9,9,9,9,9

#define MATRIX4MAPID    "matrix4[10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10]:"
#define MATRIX4VALUE    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10



KeyedArchiveTest::KeyedArchiveTest()
: TestTemplate<KeyedArchiveTest>("KeyedArchiveTest")
, archiveToSave( new KeyedArchive() )
, loadedArchive( new KeyedArchive() )
{
	//RegisterFunction(this, &KeyedArchiveTest::WriteArchive, "WriteArchive", NULL);
	RegisterFunction(this, &KeyedArchiveTest::LoadOldArchive, "LoadOldArchive", NULL);
    RegisterFunction(this, &KeyedArchiveTest::TestArchiveAccordingDefines, "TestArchiveAccordingDefines", NULL);
    RegisterFunction(this, &KeyedArchiveTest::LoadNewArchive, "LoadNewArchive", NULL);
    RegisterFunction(this, &KeyedArchiveTest::TestArchiveAccordingDefines, "TestArchiveAccordingDefines", NULL);
}

void KeyedArchiveTest::LoadResources()
{
    FillArchive(archiveToSave.Get());
}


void KeyedArchiveTest::UnloadResources()
{
}

void KeyedArchiveTest::WriteArchive(PerfFuncData * data)
{
    bool written = false;

    written = archiveToSave->Save(FILEPATHNEWFORMAT);

    TEST_VERIFY(false != written);
}

void KeyedArchiveTest::LoadOldArchive(PerfFuncData * data)
{
    bool loaded = false;
    
    loadedArchive->DeleteAllKeys();

    loaded = loadedArchive->Load(FILEPATHOLDFORMAT);
    
    TEST_VERIFY(false != loaded);
}

void KeyedArchiveTest::LoadNewArchive(PerfFuncData * data)
{
    bool loaded = false;
    
    loadedArchive->DeleteAllKeys();
       
    loaded = loadedArchive->Load(FILEPATHNEWFORMAT);
    
    TEST_VERIFY(false != loaded);
}

void KeyedArchiveTest::TestArchiveAccordingDefines(PerfFuncData * data)
{
    KeyedArchive* pWorkingKA = loadedArchive.Get();
    
    TEST_VERIFY(pWorkingKA->GetBool (BOOLMAPID)==BOOLVALUE);
    TEST_VERIFY(pWorkingKA->GetInt32(INT32MAPID)==INT32VALUE);

    TEST_VERIFY(pWorkingKA->GetUInt32(UINT32MAPID)==UINT32VALUE);
    TEST_VERIFY(FLOAT_EQUAL(pWorkingKA->GetFloat(FLOATMAPID),FLOATVALUE));
    TEST_VERIFY(pWorkingKA->GetString(STRINGMAPID)==STRINGVALUE);
    TEST_VERIFY(pWorkingKA->GetWideString(WSTRINGMAPID)==WSTRINGVALUE);
 
    uint8 arr[] = {BYTEARRVALUE};
    
    int size = pWorkingKA->GetByteArraySize(BYTEARRMAPID);
    const uint8* pByteArr = pWorkingKA->GetByteArray(BYTEARRMAPID);
    for (int i = 0; i<size; ++i)
    {
        TEST_VERIFY(*pByteArr==arr[i]);
        pByteArr++;
    }
    
    VariantType varType;
    varType.SetInt32(VARTYPEVALUE);
    
    TEST_VERIFY(*pWorkingKA->GetVariant(VARTYPEMAPID)==varType);
    
    int64 testInt64 = pWorkingKA->GetInt64(INT64MAPID,0);
    uint64 testUInt64 = pWorkingKA->GetUInt64(UINT64MAPID,0);
    
    if(testInt64 != 0)
        TEST_VERIFY( testInt64 == INT64VALUE);

    if(testUInt64 != 0)
        TEST_VERIFY(testUInt64 == UINT64VALUE);
    
    Vector2 vect2Value(VECTOR2VALUE);
    Vector3 vect3Value(VECTOR3VALUE);
    Vector4 vect4Value(VECTOR4VALUE);
    
    Vector2 vect2DefValue(0,0);
    Vector3 vect3DefValue(0,0,0);
    Vector4 vect4DefValue(0,0,0,0);
    
    const Vector2 & testV2 = pWorkingKA->GetVector2(VECTOR2MAPID, vect2DefValue);
    const Vector3 & testV3 = pWorkingKA->GetVector3(VECTOR3MAPID, vect3DefValue);
    const Vector4 & testV4 = pWorkingKA->GetVector4(VECTOR4MAPID, vect4DefValue);
    
    if(testV2 != vect2DefValue)
        TEST_VERIFY(testV2 == vect2Value);
    
    if(testV3 != vect3DefValue)
        TEST_VERIFY(testV3 == vect3Value);
    
    if(testV4 != vect4DefValue)
        TEST_VERIFY(testV4 == vect4Value);
    
    Matrix2 matrix2Value(MATRIX2VALUE);
    Matrix3 matrix3Value(MATRIX3VALUE);
    Matrix4 matrix4Value(MATRIX4VALUE);

    Matrix2 matrix2DefValue(0,0,0,0);
    Matrix3 matrix3DefValue(0,0,0,0,0,0,0,0,0);
    Matrix4 matrix4DefValue(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    
    const Matrix2 & testM2 = pWorkingKA->GetMatrix2(MATRIX2MAPID, matrix2DefValue);
    const Matrix3 & testM3 = pWorkingKA->GetMatrix3(MATRIX3MAPID, matrix3DefValue);
    const Matrix4 & testM4 = pWorkingKA->GetMatrix4(MATRIX4MAPID, matrix4DefValue);
    
    if( testM2 != matrix2DefValue )
        TEST_VERIFY( testM2 == matrix2Value);

    if( testM3 != matrix3DefValue )
        TEST_VERIFY( testM3 == matrix3Value);

    if( testM4 != matrix4DefValue )
        TEST_VERIFY( testM4 == matrix4Value);
}


void KeyedArchiveTest::FillArchive(KeyedArchive *arch)
{
    arch->SetBool(BOOLMAPID, BOOLVALUE);
    arch->SetInt32(INT32MAPID, INT32VALUE);
    arch->SetUInt32(UINT32MAPID, UINT32VALUE);
    arch->SetFloat(FLOATMAPID, FLOATVALUE);
    arch->SetString(STRINGMAPID, STRINGVALUE);
    arch->SetWideString(WSTRINGMAPID, WSTRINGVALUE);
    
    uint8 byteArr[] = {BYTEARRVALUE};
    arch->SetByteArray(BYTEARRMAPID, byteArr, BYTEARRLENGTH);
    
    VariantType varType;
    varType.SetInt32(VARTYPEVALUE);
    
    arch->SetVariant(VARTYPEMAPID, varType);
    
    
    int64   int64Value = INT64VALUE;
    uint64  uint64Value= UINT64VALUE;
    
    arch->SetInt64(INT64MAPID, int64Value);
    arch->SetUInt64(UINT64MAPID, uint64Value);
    
    Vector2 vect2Value(VECTOR2VALUE);
    Vector3 vect3Value(VECTOR3VALUE);
    Vector4 vect4Value(VECTOR4VALUE);
    
    arch->SetVector2(VECTOR2MAPID, vect2Value);
    arch->SetVector3(VECTOR3MAPID, vect3Value);
    arch->SetVector4(VECTOR4MAPID, vect4Value);
    
    Matrix2 matrix2Value(MATRIX2VALUE);
    Matrix3 matrix3Value(MATRIX3VALUE);
    Matrix4 matrix4Value(MATRIX4VALUE);
    
    arch->SetMatrix2(MATRIX2MAPID, matrix2Value);
    arch->SetMatrix3(MATRIX3MAPID, matrix3Value);
    arch->SetMatrix4(MATRIX4MAPID, matrix4Value);
}


