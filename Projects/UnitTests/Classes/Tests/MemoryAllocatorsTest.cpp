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

using namespace DAVA;

class ObjectWithNDOverload
{
public:
    Vector3 position;
    Vector3 direction;
    Quaternion orientation;

    static void * operator new(size_t size);
    static void operator delete(void * pointer, size_t size);
    static FixedSizePoolAllocator pool;
};

FixedSizePoolAllocator ObjectWithNDOverload::pool(sizeof(ObjectWithNDOverload), 64);

void * ObjectWithNDOverload::operator new(size_t size)
{
    return pool.New();
}

void ObjectWithNDOverload::operator delete(void * pointer, size_t size)
{
    return pool.Delete(pointer);
}

class ObjectWithoutNDOverload
{
public:
    Vector3 position;
    Vector3 direction;
    Quaternion orientation;

};

DAVA_TESTCLASS(MemoryAllocatorsTest)
{
    DAVA_TEST(PoolAllocatorTest)
    {
        // 32 bytes block, 64 elements
        FixedSizePoolAllocator pool(32, 64);

        uint8 * pointers[128];

        for (uint32 k = 0; k < 128; ++k)
        {
            pointers[k] = (uint8*)pool.New();
            //Logger::Debug("ptr: %p", pointers[k]);
        }

        for (uint32 k = 1; k < 128; ++k)
        {
            if (k != 64)
                TEST_VERIFY(pointers[k] == pointers[k - 1] + 32);
            //        if (k != 64)
            //        if (pointers[k] != pointers[k - 1] + 32)
            //            Logger::Debug("Allocator error");
        }

        for (uint32 k = 0; k < 128; ++k)
        {
            pool.Delete(pointers[k]);
        }
    }

    DAVA_TEST(PoolAllocatorNewDeleteTest)
    {
        ObjectWithNDOverload * object1 = new ObjectWithNDOverload;
        SafeDelete(object1);

        ObjectWithNDOverload * object2 = new ObjectWithNDOverload;
        SafeDelete(object2);
    }
};
