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

#include <thread>

using namespace DAVA;

DAVA_TESTCLASS(AtomicTest)
{
    DAVA_TEST(OperationTest)
    {
        const int theGreatestNumber = 42;
        const int theRandomNumber = 7;
        
        Atomic<char> atom_char;
        Atomic<short> atom_short;
        Atomic<int> atom_int;
        Atomic<int64> atom_int64;

        //ititial values
        TEST_VERIFY(atom_char == 0);
        TEST_VERIFY(atom_short == 0);
        TEST_VERIFY(atom_int == 0);
        TEST_VERIFY(atom_int64 == 0);

        //assigning
        atom_char = theGreatestNumber;
        atom_short = theGreatestNumber;
        atom_int = theGreatestNumber;
        atom_int64 = theGreatestNumber;

        TEST_VERIFY(atom_char == theGreatestNumber);
        TEST_VERIFY(atom_short == theGreatestNumber);
        TEST_VERIFY(atom_int == theGreatestNumber);
        TEST_VERIFY(atom_int64 == theGreatestNumber);

        //pre- & post- increment/decrement
        TEST_VERIFY(++atom_char == theGreatestNumber + 1);
        TEST_VERIFY(atom_char++ == theGreatestNumber + 1);
        TEST_VERIFY(atom_char == theGreatestNumber + 2);
        TEST_VERIFY(--atom_char == theGreatestNumber + 1);
        TEST_VERIFY(atom_char-- == theGreatestNumber + 1);
        TEST_VERIFY(atom_char == theGreatestNumber);

        TEST_VERIFY(++atom_short == theGreatestNumber + 1);
        TEST_VERIFY(atom_short++ == theGreatestNumber + 1);
        TEST_VERIFY(atom_short == theGreatestNumber + 2);
        TEST_VERIFY(--atom_short == theGreatestNumber + 1);
        TEST_VERIFY(atom_short-- == theGreatestNumber + 1);
        TEST_VERIFY(atom_short == theGreatestNumber);

        TEST_VERIFY(++atom_int == theGreatestNumber + 1);
        TEST_VERIFY(atom_int++ == theGreatestNumber + 1);
        TEST_VERIFY(atom_int == theGreatestNumber + 2);
        TEST_VERIFY(--atom_int == theGreatestNumber + 1);
        TEST_VERIFY(atom_int-- == theGreatestNumber + 1);
        TEST_VERIFY(atom_int == theGreatestNumber);

        TEST_VERIFY(++atom_int64 == theGreatestNumber + 1);
        TEST_VERIFY(atom_int64++ == theGreatestNumber + 1);
        TEST_VERIFY(atom_int64 == theGreatestNumber + 2);
        TEST_VERIFY(--atom_int64 == theGreatestNumber + 1);
        TEST_VERIFY(atom_int64-- == theGreatestNumber + 1);
        TEST_VERIFY(atom_int64 == theGreatestNumber);

        //Swap
        TEST_VERIFY(atom_char.Swap(0) == theGreatestNumber);
        TEST_VERIFY(atom_char != theGreatestNumber);
        TEST_VERIFY(atom_short.Swap(0) == theGreatestNumber);
        TEST_VERIFY(atom_short != theGreatestNumber);
        TEST_VERIFY(atom_int.Swap(0) == theGreatestNumber);
        TEST_VERIFY(atom_int != theGreatestNumber);
        TEST_VERIFY(atom_int64.Swap(0) == theGreatestNumber);
        TEST_VERIFY(atom_int64 != theGreatestNumber);

        //CAS
        atom_char = theRandomNumber;
        atom_short = theRandomNumber;
        atom_int = theRandomNumber;
        atom_int64 = theRandomNumber;
        
        TEST_VERIFY(atom_char.CompareAndSwap(1, 2) == false);
        TEST_VERIFY(atom_char.CompareAndSwap(theRandomNumber, theGreatestNumber));
        TEST_VERIFY(atom_char == theGreatestNumber);
        TEST_VERIFY(atom_short.CompareAndSwap(1, 2) == false);
        TEST_VERIFY(atom_short.CompareAndSwap(theRandomNumber, theGreatestNumber));
        TEST_VERIFY(atom_short == theGreatestNumber);
        TEST_VERIFY(atom_int.CompareAndSwap(1, 2) == false);
        TEST_VERIFY(atom_int.CompareAndSwap(theRandomNumber, theGreatestNumber));
        TEST_VERIFY(atom_int == theGreatestNumber);
        TEST_VERIFY(atom_int64.CompareAndSwap(1, 2) == false);
        TEST_VERIFY(atom_int64.CompareAndSwap(theRandomNumber, theGreatestNumber));
        TEST_VERIFY(atom_int64 == theGreatestNumber);
    }

    DAVA_TEST(MultiThreadedEnvironmentTest)
    {
        const unsigned threadCount = std::thread::hardware_concurrency();
        const unsigned cycles = 1000000;
        const unsigned targetNumber = cycles * threadCount;
        Atomic<unsigned> resultNumber {0U};

        Vector<std::thread> threads(threadCount);
        for (auto& x : threads)
        {
            x = std::thread([&]
            { 
                for (size_t i = 0; i < cycles; ++i)
                {
                    resultNumber++;
                }
            });
        }

        for (auto& x : threads) { x.join(); }
        TEST_VERIFY(resultNumber == targetNumber);
    }
};
