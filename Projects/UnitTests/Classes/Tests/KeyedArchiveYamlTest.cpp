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

#include "DAVAEngine.h"

#include "Infrastructure/GameCore.h"
#include "Infrastructure/NewTestFramework.h"

#include "Math/Math2D.h"

using namespace DAVA;

#define FILE_PATH String("~res:/KeyedArchives/keyed_archive_original.yaml")
#define GENERATED_FILE_PATH String("KeyedArchives/keyed_archive_created.yaml")
//#define GENERATED_FILE_PATH "~res:/KeyedArchives/keyed_archive_created.yaml"
//#define GENERATED_FILE_PATH "/Users/user/Documents/work/gitHub/dava.framework/Projects/UnitTests/Data/KeyedArchives/keyed_archive_created.yaml"

#define BOOLMAPID       "mapNamebool"
#define INT32MAPID      "mapNameint32"
#define UINT32MAPID     "mapNameUInt32"
#define FLOATMAPID      "mapNamefloat"
#define STRINGMAPID     "mapNameString"
#define WSTRINGMAPID    "mapNameWideString"
#define BYTEARRMAPID    "mapNameByteArrey"
#define INT64MAPID      "mapNameint64"
#define UINT64MAPID     "mapNameUInt64"
#define VECTOR2MAPID    "mapNamevector2"
#define VECTOR3MAPID    "mapNameVector3"
#define VECTOR4MAPID    "mapNameVector4"
#define MATRIX2MAPID    "mapNameMatrix2"
#define MATRIX3MAPID    "mapNameMatrix3"
#define MATRIX4MAPID    "mapNameMatrix4"
#define KEYEDARCHMAPID  "mapNameKArch"

DAVA_TESTCLASS(KeyedArchiveYamlTest)
{
    RefPtr<KeyedArchive> loadedArchive;

    KeyedArchiveYamlTest()
        : loadedArchive(new KeyedArchive())
    {}

    DAVA_TEST(TestFunction)
    {
        bool loaded = false;

        loadedArchive->DeleteAllKeys();

        loaded = loadedArchive->LoadFromYamlFile(FILE_PATH);
        TEST_VERIFY(false != loaded);

        FilePath documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
        FilePath generatedPath = documentsPath + GENERATED_FILE_PATH;

        FileSystem::Instance()->CreateDirectory(generatedPath.GetDirectory(), true);

        loadedArchive->SaveToYamlFile(generatedPath);

        ScopedPtr<KeyedArchive> loadedArchiveFromGeneratedFile(new KeyedArchive());
        loaded = loadedArchiveFromGeneratedFile->LoadFromYamlFile(generatedPath);

        TEST_VERIFY(false != loaded);

        TEST_VERIFY(*loadedArchive->GetVariant(BOOLMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(BOOLMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(INT32MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(INT32MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(UINT32MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(UINT32MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(FLOATMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(FLOATMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(STRINGMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(STRINGMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(WSTRINGMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(WSTRINGMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(BYTEARRMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(BYTEARRMAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(INT64MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(INT64MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(UINT64MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(UINT64MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(VECTOR2MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(VECTOR2MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(VECTOR3MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(VECTOR3MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(VECTOR4MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(VECTOR4MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(MATRIX2MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(MATRIX2MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(MATRIX3MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(MATRIX3MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(MATRIX4MAPID) == *loadedArchiveFromGeneratedFile->GetVariant(MATRIX4MAPID));
        TEST_VERIFY(*loadedArchive->GetVariant(KEYEDARCHMAPID) == *loadedArchiveFromGeneratedFile->GetVariant(KEYEDARCHMAPID));
    }
};
