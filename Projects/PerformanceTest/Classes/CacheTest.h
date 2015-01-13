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



#ifndef __CACHE_TEST_H__
#define __CACHE_TEST_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class CacheTest: public TestTemplate<CacheTest>
{
protected:
    ~CacheTest(){}
public:
	CacheTest(const String &testName);
    
	virtual void LoadResources();
	virtual void UnloadResources();
        
    int32 elementCount;
    
    Vector3 * array0;
    Vector3 * array1;
    float32 * arrayResult;
    float32 * correctResult;
    
    Vector3 * crossResult;
    Vector3 ** arrayRandom0;
    Vector3 ** arrayRandom1;

    Matrix4 * matrixes0;
    Matrix4 * matrixes1;
    Matrix4 * matrixesResult;
    Matrix4 * neonMatrixesResult;
    
    
protected:
    void SequentionalDotProductTest(PerfFuncData * data);
    void RandomDotProductTest(PerfFuncData * data);

    void SequentionalCrossProductTest(PerfFuncData * data);
    void RandomCrossProductTest(PerfFuncData * data);
    
    void DefaultMatrixMul(PerfFuncData * data);
    void NeonMatrixMul(PerfFuncData * data);


};


#endif // __CACHE_TEST_H__
