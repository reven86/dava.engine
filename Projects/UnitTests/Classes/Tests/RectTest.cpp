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

#include "Math/Rect.h"

using namespace DAVA;

DAVA_TESTCLASS(RectTest){
    //this test from Qt: void tst_QRect::containsRectF_data()
    DAVA_TEST(RectContainsTest){
    TEST_VERIFY(Rect(0.0f, 0.0f, 10.0f, 10.0f).RectContains(Rect(2.0f, 2.0f, 6.0f, 6.0f)));
TEST_VERIFY(Rect(0.0f, 0.0f, 10.0f, 10.0f).RectContains(Rect(0.0f, 0.0f, 10.0f, 10.0f)));
TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectContains(Rect(2.0f, 2.0f, 10.0f, 10.0f)));
TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectContains(Rect(20.0f, 20.0f, 10.0f, 10.0f)));

TEST_VERIFY(!Rect(10.0f, 10.0f, -10.0f, -10.0f).RectContains(Rect(2.0f, 2.0f, 6.0f, 6.0f)));
TEST_VERIFY(!Rect(10.0f, 10.0f, -10.0f, -10.0f).RectContains(Rect(0.0f, 0.0f, 10.0f, 10.0f)));
TEST_VERIFY(!Rect(10.0f, 10.0f, -10.0f, -10.0f).RectContains(Rect(2.0f, 2.0f, 10.0f, 10.0f)));
TEST_VERIFY(!Rect(10.0f, 10.0f, -10.0f, -10.0f).RectContains(Rect(20.0f, 20.0f, 10.0f, 10.0f)));

TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectContains(Rect(8.0f, 8.0f, -6.0f, -6.0f)));
TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectContains(Rect(10.0f, 10.0f, -10.0f, -10.0f)));
TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectContains(Rect(12.0f, 12.0f, -10.0f, -10.0f)));
TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectContains(Rect(30.0f, 30.0f, -10.0f, -10.0f)));

TEST_VERIFY(Rect(-1.0f, -1.0f, 10.0f, 10.0f).RectContains(Rect()));
TEST_VERIFY(!Rect().RectContains(Rect(0.0f, 0.0f, 10.0f, 10.0f)));
TEST_VERIFY(Rect().RectContains(Rect()));
}

//this test from Qt: void tst_QRect::intersectsRectF_data()
DAVA_TEST(RectIntersectsTest)
{
    TEST_VERIFY(Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(2.0f, 2.0f, 6.0f, 6.0f)));
    TEST_VERIFY(Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(0.0f, 0.0f, 10.0f, 10.0f)));
    TEST_VERIFY(Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(2.0f, 2.0f, 10.0f, 10.0f)));
    TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(20.0f, 20.0f, 10.0f, 10.0f)));

    TEST_VERIFY(!Rect(10.0f, 10.0f, -10.0f, -10.0f).RectIntersects(Rect(2.0f, 2.0f, 6.0f, 6.0f)));
    TEST_VERIFY(!Rect(10.0f, 10.0f, -10.0f, -10.0f).RectIntersects(Rect(0.0f, 0.0f, 10.0f, 10.0f)));
    TEST_VERIFY(!Rect(10.0f, 10.0f, -10.0f, -10.0f).RectIntersects(Rect(2.0f, 2.0f, 10.0f, 10.0f)));
    TEST_VERIFY(!Rect(10.0f, 10.0f, -10.0f, -10.0f).RectIntersects(Rect(20.0f, 20.0f, 10.0f, 10.0f)));

    TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(8.0f, 8.0f, -6.0f, -6.0f)));
    TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(10.0f, 10.0f, -10.0f, -10.0f)));
    TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(12.0f, 12.0f, -10.0f, -10.0f)));
    TEST_VERIFY(!Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(30.0f, 30.0f, -10.0f, -10.0f)));

    TEST_VERIFY(Rect(15.0f, 15.0f, 0.0f, 0.0f).RectIntersects(Rect(10.0f, 10.0f, 10.0f, 10.0f)));
    TEST_VERIFY(Rect(10.0f, 10.0f, 10.0f, 10.0f).RectIntersects(Rect(15.0f, 15.0f, 0.0f, 0.0f)));
    TEST_VERIFY(Rect().RectIntersects(Rect()));

    TEST_VERIFY(Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(10.0f, 10.0f, 10.0f, 10.0f)));
    TEST_VERIFY(Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(0.0f, 10.0f, 10.0f, 10.0f)));
    TEST_VERIFY(Rect(0.0f, 0.0f, 10.0f, 10.0f).RectIntersects(Rect(10.0f, 0.0f, 10.0f, 10.0f)));
}
}
;
