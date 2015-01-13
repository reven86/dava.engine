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



#include "CacheTest.h"
#include "Math/Neon/NeonMath.h"

CacheTest::CacheTest(const String &testName)
    :   TestTemplate<CacheTest>(testName)
{
    RegisterFunction(this, &CacheTest::SequentionalDotProductTest, "SequentionalDotProductTest", 1,  NULL);
    RegisterFunction(this, &CacheTest::RandomDotProductTest, "RandomDotProductTest", 1,  NULL);
    RegisterFunction(this, &CacheTest::SequentionalCrossProductTest, "SequentionalCrossProductTest", 1,  NULL);
    RegisterFunction(this, &CacheTest::RandomCrossProductTest, "RandomCrossProductTest", 1,  NULL);
    RegisterFunction(this, &CacheTest::DefaultMatrixMul, "DefaultMatrixMul", 1,  NULL);
#ifdef _ARM_ARCH_7
    RegisterFunction(this, &CacheTest::NeonMatrixMul, "NeonMatrixMul", 1,  NULL);
#endif //#ifdef _ARM_ARCH_7
}


void CacheTest::LoadResources()
{
    elementCount = 200000;
    
    array0 = new Vector3[elementCount];
    array1 = new Vector3[elementCount];
    arrayResult = new float32[elementCount];
    correctResult = new float32[elementCount];
    crossResult = new Vector3[elementCount];
    
    for (int32 k = 0; k < elementCount; ++k)
    {
        array0[k] = Vector3(Random::Instance()->RandFloat(), Random::Instance()->RandFloat(), Random::Instance()->RandFloat());
        array1[k] = Vector3(Random::Instance()->RandFloat(), Random::Instance()->RandFloat(), Random::Instance()->RandFloat());
        correctResult[k] = DotProduct(array0[k], array1[k]);
    }
    
    arrayRandom0 = new Vector3*[elementCount];
    arrayRandom1 = new Vector3*[elementCount];

    // Random fill
    for (int32 k = 0; k < elementCount; ++k)
    {
        arrayRandom0[k] = 0;
        arrayRandom1[k] = 0;
    }

    for (int32 k = 0; k < elementCount; ++k)
    {
        int32 index = rand() % elementCount;
        if (!arrayRandom0[index])
            arrayRandom0[index] = new Vector3(Random::Instance()->RandFloat(), Random::Instance()->RandFloat(), Random::Instance()->RandFloat());
        
        int32 index2 = rand() % elementCount;
        if (!arrayRandom1[index2])
            arrayRandom1[index2] = new Vector3(Random::Instance()->RandFloat(), Random::Instance()->RandFloat(), Random::Instance()->RandFloat());    
    }
    
    for (int32 k = 0; k < elementCount; ++k)
    {
        if (!arrayRandom0[k])
            arrayRandom0[k] = new Vector3(Random::Instance()->RandFloat(), Random::Instance()->RandFloat(), Random::Instance()->RandFloat());
        
        if (!arrayRandom1[k])
            arrayRandom1[k] = new Vector3(Random::Instance()->RandFloat(), Random::Instance()->RandFloat(), Random::Instance()->RandFloat());    
    }
    
    for (int32 k = 0; k < elementCount; ++k)
    {
//      Good idea for data optimization
//        arrayRandom0[k] = new Vector3();
//        arrayRandom1[k] = new Vector3();
    }
    
    
    matrixes0 = new Matrix4[elementCount];
    matrixes1 = new Matrix4[elementCount];
    matrixesResult = new Matrix4[elementCount];
    neonMatrixesResult = new Matrix4[elementCount];
    
    for (int32 k = 0; k < elementCount; ++k)
    {
        matrixes0[k] = Matrix4::IDENTITY;
        matrixes1[k] = Matrix4::IDENTITY;

        for (int32 i = 0; i < 4; ++i)
            for (int32 j = 0; j < 4; ++j)
            {
                matrixes0[k]._data[i][j] = rand();
                matrixes1[k]._data[i][j] = rand();
            } 
    }
}

void CacheTest::UnloadResources()
{
    for (int32 k = 0; k < elementCount; ++k)
    {
        if (correctResult[k] != arrayResult[k])
        {
            Logger::Debug("Function calculated results wrongly");
            break;
        }
    }
    for (int32 k = 0; k < elementCount; ++k)
    {
        if (matrixesResult[k]._data[1][3] != neonMatrixesResult[k]._data[1][3])
        {
            Logger::Debug("Neon Function calculated results wrongly");
            break;
        }
    }
    
    SafeDeleteArray(array0);
    SafeDeleteArray(array1);
    SafeDeleteArray(arrayResult);
    SafeDeleteArray(correctResult);
    SafeDeleteArray(crossResult);

    
    for (int32 k = 0; k < elementCount; ++k)
    {
        SafeDelete(arrayRandom0[k]);
        SafeDelete(arrayRandom1[k]);
    }
    SafeDeleteArray(arrayRandom0);
    SafeDeleteArray(arrayRandom1);
    
    
    SafeDeleteArray(matrixes0);
    SafeDeleteArray(matrixes1);
    SafeDeleteArray(matrixesResult);
    SafeDeleteArray(neonMatrixesResult);
    
}

void CacheTest::SequentionalDotProductTest(PerfFuncData * data)
{
    for (int32 k = 0; k < elementCount; ++k)
    {
        arrayResult[k] = DotProduct(array0[k], array1[k]);
    }
}

void CacheTest::RandomDotProductTest(PerfFuncData * data)
{
    for (int32 k = 0; k < elementCount; ++k)
    {
        arrayResult[k] = DotProduct(*arrayRandom0[k], *arrayRandom1[k]);
    }
}

void CacheTest::SequentionalCrossProductTest(PerfFuncData * data)
{
    for (int32 k = 0; k < elementCount; ++k)
    {
        crossResult[k] = CrossProduct(array0[k], array1[k]);
    }
}

void CacheTest::RandomCrossProductTest(PerfFuncData * data)
{
    for (int32 k = 0; k < elementCount; ++k)
    {
        crossResult[k] = CrossProduct(*arrayRandom0[k], *arrayRandom1[k]);
    }
}

void CacheTest::DefaultMatrixMul(PerfFuncData * data)
{
    for (int32 k = 0; k < elementCount; ++k)
    {
        matrixesResult[k] = matrixes0[k] * matrixes1[k]; 
    }
}

void CacheTest::NeonMatrixMul(PerfFuncData * data)
{
#ifdef _ARM_ARCH_7
    for (int32 k = 0; k < elementCount; ++k)
    {
        NEON_Matrix4Mul(matrixes0[k].data, matrixes1[k].data, neonMatrixesResult[k].data);     
    }
#endif //#ifdef _ARM_ARCH_7
    
}

