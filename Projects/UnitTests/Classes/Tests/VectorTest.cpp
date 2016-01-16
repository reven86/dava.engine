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
#include "UnitTests/UnitTests.h"
#include "Math/Vector.h"

using namespace DAVA;

DAVA_TESTCLASS(VectorTest){
    DAVA_TEST(Vector4ToVector3Test){
    Vector4 mutableSource(0.1f, 0.2f, 0.3f, 0.4f);
Vector3& mutableVector3 = mutableSource.GetVector3();
TEST_VERIFY((mutableVector3.x == mutableSource.x) && (mutableVector3.y == mutableSource.y) && (mutableVector3.z == mutableSource.z))

const Vector4 immutableSource(0.3f, 0.2f, 0.1f, 0.0f);
const Vector3& immutableVector3 = immutableSource.GetVector3();
TEST_VERIFY((immutableVector3.x == immutableSource.x) && (immutableVector3.y == immutableSource.y) && (immutableVector3.z == immutableSource.z))
}
}
;
