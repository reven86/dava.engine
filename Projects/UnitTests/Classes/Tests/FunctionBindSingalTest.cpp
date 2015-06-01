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

#include "Base/Function.h"
#include "Base/Bind.h"
#include "Base/Signal2.h"

using namespace DAVA;

// =======================================================================================================================================
// =======================================================================================================================================
// =======================================================================================================================================

static String functionBindSignalResultString;

int staticFn0() { return 100; }
int staticFn1(int i1) { return i1; }
int staticFn2(int i1, int i2) { return i1 + i2; }
int staticFn3(int i1, int i2, int i3) { return i1 + i2 + i3; }
int staticFn4(int i1, int i2, int i3, int i4) { return i1 * i2 + i3 * i4; }
int staticFn5(int i1, int i2, int i3, int i4, int i5) { return i1 * (i2 + i3) - i4 * i5; }
int staticFn6(int i1, int i2, int i3, int i4, int i5, int i6) { return i1 * (i2 + i3) - i4 * (i5 + i6); }
int staticFn7(int i1, int i2, int i3, int i4, int i5, int i6, int i7) { return i1 * (i2 + i3) - i4 * (i5 + i6) + i7; }
int staticFn8(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8) { return i1 * (i2 + i3) - i4 * (i5 + i6) + i7 * i8; }

void* exStaticFnV(void *a) { return a; }
void* exStaticFnVV(void **a) { return *a; }
const char* exStaticFnCC(const char *a) { return a; }
const char* exStaticFnCCC(const char **a) { return *a; }

struct A
{
    int classFn0() { return 100; }
    int classFn1(int i1) { return i1; }
    int classFn2(int i1, int i2) { return i1 + i2; }
    int classFn3(int i1, int i2, int i3) { return i1 + i2 + i3; }
    int classFn4(int i1, int i2, int i3, int i4) { return i1 * i2 + i3 * i4; }
    int classFn5(int i1, int i2, int i3, int i4, int i5) { return i1 * (i2 + i3) - i4 * i5; }
    int classFn6(int i1, int i2, int i3, int i4, int i5, int i6) { return i1 * (i2 + i3) - i4 * (i5 + i6); }
    int classFn7(int i1, int i2, int i3, int i4, int i5, int i6, int i7) { return i1 * (i2 + i3) - i4 * (i5 + i6) + i7; }
    int classFn8(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8) { return i1 * (i2 + i3) - i4 * (i5 + i6) + (i7 * i8); }

    int classFn0_const() const { return 100; }
    int classFn1_const(int i1) const { return i1; }
    int classFn2_const(int i1, int i2) const { return i1 + i2; }
    int classFn3_const(int i1, int i2, int i3) const { return i1 + i2 + i3; }
    int classFn4_const(int i1, int i2, int i3, int i4) const { return i1 * i2 + i3 * i4; }
    int classFn5_const(int i1, int i2, int i3, int i4, int i5) const { return i1 * (i2 + i3) - i4 * i5; }
    int classFn6_const(int i1, int i2, int i3, int i4, int i5, int i6) { return i1 * (i2 + i3) - i4 * (i5 + i6); }
    int classFn7_const(int i1, int i2, int i3, int i4, int i5, int i6, int i7) { return i1 * (i2 + i3) - i4 * (i5 + i6) + i7; }
    int classFn8_const(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8) { return i1 * (i2 + i3) - i4 * (i5 + i6) + (i7 * i8); }

    int incomingFunction(Function<int()> fn) { return fn(); }
    int incomingFunctionRef(Function<int()> &fn) { return fn(); }
    int incomingFunctionConstRef(const Function<int()> &fn) { return fn(); }

    void setV(int value) { v = value; }
    int getV() { return v; }

    int v;
};

struct B : public A
{ 
    A* exClassFn(A* a) { return a; }
    const A* exClassFn1(const A* a) { return a; }
    A* exClassFn2(A** a) { return *a; }
    const A* exClassFn3(const A** a) { return *a; }
};

struct M
{
    char c_[128];
};

struct V
{
    virtual int f2virt(int i, long j) = 0;
    char c_[2];
};

struct C : public M, virtual public V, virtual public A
{
    virtual void f2() { }
    virtual int f2defvirt(int i, long j) { return static_cast<int>(i + j + 1); }
    int f2def(int i, long j) { return static_cast<int>(i + j + 3); }
    virtual int f2virt(int i, long j) { return static_cast<int>(i + j + 2); }
};

// =======================================================================================================================================
// =======================================================================================================================================
// =======================================================================================================================================

DAVA_TESTCLASS(FunctionBindSignalTest)
{
    DAVA_TEST(TestFunction)
    {
        // ==================================================================================
        // common functions testing
        // ==================================================================================
        Function<int()> static_f0(&staticFn0);
        Function<int(int)> static_f1(&staticFn1);
        Function<int(int, int)> static_f2(&staticFn2);
        Function<int(int, int, int)> static_f3(&staticFn3);
        Function<int(int, int, int, int)> static_f4(&staticFn4);
        Function<int(int, int, int, int, int)> static_f5(&staticFn5);
        Function<int(int, int, int, int, int, int)> static_f6(&staticFn6);
        Function<int(int, int, int, int, int, int, int)> static_f7(&staticFn7);
        Function<int(int, int, int, int, int, int, int, int)> static_f8(&staticFn8);

        TEST_VERIFY(static_f0() == staticFn0());
        TEST_VERIFY(static_f1(3) == staticFn1(3));
        TEST_VERIFY(static_f2(3, 8) == staticFn2(3, 8));
        TEST_VERIFY(static_f3(3, 8, 5) == staticFn3(3, 8, 5));
        TEST_VERIFY(static_f4(3, 8, 5, 0) == staticFn4(3, 8, 5, 0));
        TEST_VERIFY(static_f5(3, 8, 5, 0, 2) == staticFn5(3, 8, 5, 0, 2));
        TEST_VERIFY(static_f6(3, 8, 5, 0, 2, 11) == staticFn6(3, 8, 5, 0, 2, 11));
        TEST_VERIFY(static_f7(3, 8, 5, 0, 2, 11, 8) == staticFn7(3, 8, 5, 0, 2, 11, 8));
        TEST_VERIFY(static_f8(3, 8, 5, 0, 2, 11, 8, 2) == staticFn8(3, 8, 5, 0, 2, 11, 8, 2));

        A a;
        Function<int(A*)> class_f0 = &A::classFn0;
        Function<int(A*, int)> class_f1 = &A::classFn1;
        Function<int(A*, int, int)> class_f2 = &A::classFn2;
        Function<int(A*, int, int, int)> class_f3 = &A::classFn3;
        Function<int(A*, int, int, int, int)> class_f4 = &A::classFn4;
        Function<int(A*, int, int, int, int, int)> class_f5 = &A::classFn5;
        Function<int(A*, int, int, int, int, int, int)> class_f6 = &A::classFn6;
        Function<int(A*, int, int, int, int, int, int, int)> class_f7 = &A::classFn7;

        Function<int(A*)> class_f0c = &A::classFn0_const;
        Function<int(A*, int)> class_f1c = &A::classFn1_const;
        Function<int(A*, int, int)> class_f2c = &A::classFn2_const;
        Function<int(A*, int, int, int)> class_f3c = &A::classFn3_const;
        Function<int(A*, int, int, int, int)> class_f4c = &A::classFn4_const;
        Function<int(A*, int, int, int, int, int)> class_f5c = &A::classFn5_const;
        Function<int(A*, int, int, int, int, int, int)> class_f6c = &A::classFn6_const;
        Function<int(A*, int, int, int, int, int, int, int)> class_f7c = &A::classFn7_const;

        TEST_VERIFY(class_f0(&a) == a.classFn0());
        TEST_VERIFY(class_f1(&a, 3) == a.classFn1(3));
        TEST_VERIFY(class_f2(&a, 3, 8) == a.classFn2(3, 8));
        TEST_VERIFY(class_f3(&a, 3, 8, 5) == a.classFn3(3, 8, 5));
        TEST_VERIFY(class_f4(&a, 3, 8, 5, 0) == a.classFn4(3, 8, 5, 0));
        TEST_VERIFY(class_f5(&a, 3, 8, 5, 0, 2) == a.classFn5(3, 8, 5, 0, 2));
        TEST_VERIFY(class_f6(&a, 3, 8, 5, 0, 2, 3) == a.classFn6(3, 8, 5, 0, 2, 3));
        TEST_VERIFY(class_f7(&a, 3, 8, 5, 0, 2, 3, 4) == a.classFn7(3, 8, 5, 0, 2, 3, 4));

        TEST_VERIFY(class_f0c(&a) == a.classFn0_const());
        TEST_VERIFY(class_f1c(&a, 3) == a.classFn1_const(3));
        TEST_VERIFY(class_f2c(&a, 3, 8) == a.classFn2_const(3, 8));
        TEST_VERIFY(class_f3c(&a, 3, 8, 5) == a.classFn3_const(3, 8, 5));
        TEST_VERIFY(class_f4c(&a, 3, 8, 5, 0) == a.classFn4_const(3, 8, 5, 0));
        TEST_VERIFY(class_f5c(&a, 3, 8, 5, 0, 2) == a.classFn5_const(3, 8, 5, 0, 2));
        TEST_VERIFY(class_f6c(&a, 3, 8, 5, 0, 2, 3) == a.classFn6_const(3, 8, 5, 0, 2, 3));
        TEST_VERIFY(class_f7c(&a, 3, 8, 5, 0, 2, 3, 4) == a.classFn7_const(3, 8, 5, 0, 2, 3, 4));

        // ==================================================================================
        // function with assigned object
        // ==================================================================================
        Function<int()>                                       object_f0(&a, &A::classFn0);
        Function<int(int)>                                    object_f1(&a, &A::classFn1);
        Function<int(int, int)>                               object_f2(&a, &A::classFn2);
        Function<int(int, int, int)>                          object_f3(&a, &A::classFn3);
        Function<int(int, int, int, int)>                     object_f4(&a, &A::classFn4);
        Function<int(int, int, int, int, int)>                object_f5(&a, &A::classFn5);
        Function<int(int, int, int, int, int, int)>           object_f6(&a, &A::classFn6);
        Function<int(int, int, int, int, int, int, int)>      object_f7(&a, &A::classFn7);
        Function<int(int, int, int, int, int, int, int, int)> object_f8(&a, &A::classFn8);

        Function<int()>                                       object_f0c(&a, &A::classFn0_const);
        Function<int(int)>                                    object_f1c(&a, &A::classFn1_const);
        Function<int(int, int)>                               object_f2c(&a, &A::classFn2_const);
        Function<int(int, int, int)>                          object_f3c(&a, &A::classFn3_const);
        Function<int(int, int, int, int)>                     object_f4c(&a, &A::classFn4_const);
        Function<int(int, int, int, int, int)>                object_f5c(&a, &A::classFn5_const);
        Function<int(int, int, int, int, int, int)>           object_f6c(&a, &A::classFn6_const);
        Function<int(int, int, int, int, int, int, int)>      object_f7c(&a, &A::classFn7_const);
        Function<int(int, int, int, int, int, int, int, int)> object_f8c(&a, &A::classFn8_const);

        TEST_VERIFY(object_f0() == a.classFn0());
        TEST_VERIFY(object_f1(3) == a.classFn1(3));
        TEST_VERIFY(object_f2(3, 8) == a.classFn2(3, 8));
        TEST_VERIFY(object_f3(3, 8, 5) == a.classFn3(3, 8, 5));
        TEST_VERIFY(object_f4(3, 8, 5, 0) == a.classFn4(3, 8, 5, 0));
        TEST_VERIFY(object_f5(3, 8, 5, 0, 2) == a.classFn5(3, 8, 5, 0, 2));
        TEST_VERIFY(object_f6(3, 8, 5, 0, 2, 3) == a.classFn6(3, 8, 5, 0, 2, 3));
        TEST_VERIFY(object_f7(3, 8, 5, 0, 2, 3, 4) == a.classFn7(3, 8, 5, 0, 2, 3, 4));
        TEST_VERIFY(object_f8(3, 8, 5, 0, 2, 3, 4, 9) == a.classFn8(3, 8, 5, 0, 2, 3, 4, 9));

        TEST_VERIFY(object_f0c() == a.classFn0_const());
        TEST_VERIFY(object_f1c(3) == a.classFn1_const(3));
        TEST_VERIFY(object_f2c(3, 8) == a.classFn2_const(3, 8));
        TEST_VERIFY(object_f3c(3, 8, 5) == a.classFn3_const(3, 8, 5));
        TEST_VERIFY(object_f4c(3, 8, 5, 0) == a.classFn4_const(3, 8, 5, 0));
        TEST_VERIFY(object_f5c(3, 8, 5, 0, 2) == a.classFn5_const(3, 8, 5, 0, 2));
        TEST_VERIFY(object_f6c(3, 8, 5, 0, 2, 3) == a.classFn6_const(3, 8, 5, 0, 2, 3));
        TEST_VERIFY(object_f7c(3, 8, 5, 0, 2, 3, 4) == a.classFn7_const(3, 8, 5, 0, 2, 3, 4));
        TEST_VERIFY(object_f8c(3, 8, 5, 0, 2, 3, 4, 9) == a.classFn8_const(3, 8, 5, 0, 2, 3, 4, 9));

        // ==================================================================================
        // MakeFunction helper testing
        // ==================================================================================
        MakeFunction(&staticFn0)();
        MakeFunction(&staticFn1)(1);
        MakeFunction(&staticFn2)(1, 2);
        MakeFunction(&staticFn3)(1, 2, 3);
        MakeFunction(&staticFn4)(1, 2, 3, 4);
        MakeFunction(&staticFn5)(1, 2, 3, 4, 5);
        MakeFunction(&staticFn6)(1, 2, 3, 4, 5, 6);
        MakeFunction(&staticFn7)(1, 2, 3, 4, 5, 6, 7);
        MakeFunction(&staticFn8)(1, 2, 3, 4, 5, 6, 7, 8);

        MakeFunction(&A::classFn0)(&a);
        MakeFunction(&A::classFn1)(&a, 1);
        MakeFunction(&A::classFn2)(&a, 1, 2);
        MakeFunction(&A::classFn3)(&a, 1, 2, 3);
        MakeFunction(&A::classFn4)(&a, 1, 2, 3, 4);
        MakeFunction(&A::classFn5)(&a, 1, 2, 3, 4, 5);
        MakeFunction(&A::classFn6)(&a, 1, 2, 3, 4, 5, 6);
        MakeFunction(&A::classFn7)(&a, 1, 2, 3, 4, 5, 6, 7);

        MakeFunction(static_f5)(1, 2, 3, 4, 5);
        MakeFunction(class_f5)(&a, 1, 2, 3, 4, 5);

        // ==================================================================================
        // inherited class testing
        // ==================================================================================
        B b;
        Function<void(B*, int)> b0_class_setV = &B::setV;
        Function<void(A*, int)> b1_class_setV = &B::setV;
        Function<int(A*)> b3_class_getV = &A::getV;
        Function<int(B*)> b4_class_getV = &B::getV;

        b0_class_setV(&b, 100); TEST_VERIFY(b.getV() == 100);
        b1_class_setV(&b, 200); TEST_VERIFY(b.getV() == 200);
        b1_class_setV(&a, 100); TEST_VERIFY(a.getV() == 100);
        b.setV(300); TEST_VERIFY(b3_class_getV(&b) == 300);
        a.setV(300); TEST_VERIFY(b3_class_getV(&a) == 300);
        b.setV(400); TEST_VERIFY(b4_class_getV(&b) == 400);

        // ==================================================================================
        // virtual functions testing
        // ==================================================================================
        C c;
        Function<void(C*)> c_f2 = &C::f2;
        Function<int(C*, int i, long j)> c_f2defvirt = &C::f2defvirt;
        Function<int(C*, int i, long j)> c_f2def = &C::f2def;
        Function<int(C*, int i, long j)> c_f2virt = &C::f2virt;

        c_f2(&c);
        TEST_VERIFY(c_f2defvirt(&c, 2000, 4454656) == c.f2defvirt(2000, 4454656));
        TEST_VERIFY(c_f2def(&c, 2000, 4454656) == c.f2def(2000, 4454656));
        TEST_VERIFY(c_f2virt(&c, 2000, 4454656) == c.f2virt(2000, 4454656));

        // ==================================================================================
        // type casting testing
        // ==================================================================================
        TEST_VERIFY(a.incomingFunction(static_f0) == staticFn0());
        TEST_VERIFY(a.incomingFunctionRef(static_f0) == staticFn0());
        TEST_VERIFY(a.incomingFunctionConstRef(static_f0) == staticFn0());
        TEST_VERIFY(a.incomingFunction(&staticFn0) == staticFn0());
        TEST_VERIFY(a.incomingFunctionConstRef(&staticFn0) == staticFn0());

        // ==================================================================================
        // operators
        // ==================================================================================
        Function<int()> null_f0;
        Function<int()> null_f0_1 = 0;

        TEST_VERIFY(null_f0 == null_f0_1);
        TEST_VERIFY(null_f0 == 0);
        TEST_VERIFY(null_f0_1 == 0);
        TEST_VERIFY(class_f0 == &A::classFn0);

        null_f0 = static_f0;
        TEST_VERIFY(null_f0() == staticFn0());
        null_f0 = 0;
        TEST_VERIFY(null_f0 == 0);

        null_f0 = object_f0;
        TEST_VERIFY(null_f0() == object_f0());

        // ==================================================================================
        // bind testing
        // ==================================================================================
        A aa;
        Function<int()> bound_create_f0 = Bind(&A::classFn0, &aa);
        Function<int()> bound_create_f1 = Bind(class_f1, &aa, 10);

        TEST_VERIFY(bound_create_f0() == a.classFn0());
        TEST_VERIFY(bound_create_f1() == class_f1(&aa, 10));

        Function<int()>   bound_f0 = Bind(class_f0, &aa);
        Function<int(A*)> bound_f0_1 = Bind(class_f0, _1);

        bound_f0();

        TEST_VERIFY(bound_f0() == class_f0(&aa));
        TEST_VERIFY(bound_f0_1(&aa) == class_f0(&aa));

        Function<int()>        bound_f1 = Bind(class_f1, &aa, 10);
        Function<int(int)>     bound_f1_1 = Bind(class_f1, &aa, _1);
        Function<int(A*)>      bound_f1_2 = Bind(class_f1, _1, 10);
        Function<int(int, A*)> bound_f1_3 = Bind(class_f1, _2, _1);

        TEST_VERIFY(bound_f1() == class_f1(&aa, 10));
        TEST_VERIFY(bound_f1_1(10) == class_f1(&aa, 10));
        TEST_VERIFY(bound_f1_2(&aa) == class_f1(&aa, 10));
        TEST_VERIFY(bound_f1_3(10, &aa) == class_f1(&aa, 10));

        Function<int()>         bound_f2 = Bind(class_f2, &aa, 10, 20);
        Function<int(A*)>       bound_f2_1 = Bind(class_f2, _1, 10, 20);
        Function<int(int, int)> bound_f2_2 = Bind(class_f2, &aa, _1, _2);
        Function<int(int, int)> bound_f2_3 = Bind(class_f2, &aa, _2, _1);
        Function<int(A*, int)>  bound_f2_4 = Bind(class_f2, _1, 10, _2);
        Function<int(A*, int)>  bound_f2_5 = Bind(class_f2, _1, _2, 20);
        Function<int(int)>      bound_f2_6 = Bind(class_f2, &aa, _1, 20);
        Function<int(int)>      bound_f2_7 = Bind(class_f2, &aa, 10, _1);

        TEST_VERIFY(bound_f2() == class_f2(&aa, 10, 20));
        TEST_VERIFY(bound_f2_1(&aa) == class_f2(&aa, 10, 20));
        TEST_VERIFY(bound_f2_2(10, 20) == class_f2(&aa, 10, 20));
        TEST_VERIFY(bound_f2_3(20, 10) == class_f2(&aa, 10, 20));
        TEST_VERIFY(bound_f2_4(&aa, 20) == class_f2(&aa, 10, 20));
        TEST_VERIFY(bound_f2_5(&aa, 10) == class_f2(&aa, 10, 20));
        TEST_VERIFY(bound_f2_6(10) == class_f2(&aa, 10, 20));
        TEST_VERIFY(bound_f2_7(20) == class_f2(&aa, 10, 20));

        Function<int(int, int, int)> bound_f3 = Bind(class_f3, &aa, _3, _2, _1);
        Function<int(int, int, int, int)> bound_f4 = Bind(class_f4, &aa, _4, _3, _2, _1);
        Function<int(int, int, int, int, int)> bound_f5 = Bind(class_f5, &aa, _5, _4, _3, _2, _1);
        Function<int(int, int, int, int, int, int)> bound_f6 = Bind(class_f6, &aa, _6, _5, _4, _3, _2, _1);
        Function<int(int, int, int, int, int, int, int)> bound_f7 = Bind(class_f7, &aa, _7, _6, _5, _4, _3, _2, _1);

        TEST_VERIFY(bound_f3(30, 20, 10) == class_f3(&aa, 10, 20, 30));
        TEST_VERIFY(bound_f4(40, 30, 20, 10) == class_f4(&aa, 10, 20, 30, 40));
        TEST_VERIFY(bound_f5(50, 40, 30, 20, 10) == class_f5(&aa, 10, 20, 30, 40, 50));
        TEST_VERIFY(bound_f6(60, 50, 40, 30, 20, 10) == class_f6(&aa, 10, 20, 30, 40, 50, 60));
        TEST_VERIFY(bound_f7(70, 60, 50, 40, 30, 20, 10) == class_f7(&aa, 10, 20, 30, 40, 50, 60, 70));

        // ==================================================================================
        // signals
        // ==================================================================================
#if 1
        Signal<void()> sig0;
        Signal<int(int, int, int)> sig3;

        Function<void()> slot0 = Bind(&A::setV, &a, 10);
        sig0.Connect(slot0);
        sig0.Emit();

        sig3.Connect(static_f3);
        sig3.Emit(10, 20, 30);
#endif

        // ==================================================================================
        // speed testing
        // ==================================================================================
#if 0
        //std::function<int()> c11_static_f0 = &staticFn0;
        //std::function<int(int, int, int, int, int, int, int, int)> c11_static_f8 = &staticFn8;

        //std::function<int(A*)> c11_class_f0 = std::mem_fn(&A::classFn0);
        //std::function<int(A*, int, int, int, int, int, int, int)> c11_class_f7 = std::mem_fn(&A::classFn7);

        //std::function<int()> c11_bound_f0 = std::bind(std::mem_fn(&A::classFn0), &a);
        //std::function<int(int, int, int, int, int, int, int)> c11_bound_f7 = std::bind(std::mem_fn(&A::classFn7), &a, std::placeholders::_7, std::placeholders::_6, std::placeholders::_5, std::placeholders::_4, std::placeholders::_3, std::placeholders::_2, std::placeholders::_1);

        int st_count = 1000000;
        uint64 time_ms;

        functionBindSignalResultString += Format("\n\nEach invoke test will be run %u times:\n\n", st_count);

        //time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { c11_static_f0(); }
        //functionBindSignalResultString += Format(" c11 static function 0: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { static_f0(); }
        functionBindSignalResultString += Format(" dava static function 0: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        //time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { c11_static_f8(1, 2, 3, 4, 5, 6, 7, 8); }
        //functionBindSignalResultString += Format(" c11 static function 8: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { static_f8(1, 2, 3, 4, 5, 6, 7, 8); }
        functionBindSignalResultString += Format(" dava static function 8: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        //time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { c11_class_f0(&a); }
        //functionBindSignalResultString += Format(" c11 class function 0: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { class_f0(&a); }
        functionBindSignalResultString += Format(" dava class function 0: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        //time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { c11_class_f7(&a, 1, 2, 3, 4, 5, 6, 7); }
        //functionBindSignalResultString += Format(" c11 class function 8: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { class_f7(&a, 1, 2, 3, 4, 5, 6, 7); }
        functionBindSignalResultString += Format(" dava class function 8: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        //time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { c11_bound_f0(); }
        //functionBindSignalResultString += Format(" bound c11 class function 0: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { bound_f0(); }
        functionBindSignalResultString += Format(" bound dava class function 0: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        //time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { c11_bound_f7(10, 20, 30, 40, 50, 60, 70); }
        //functionBindSignalResultString += Format(" bound c11 class function 7: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { bound_f7(10, 20, 30, 40, 50, 60, 70); }
        functionBindSignalResultString += Format(" bound dava class function 7: %llu ms\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);

        time_ms = DAVA::SystemTimer::Instance()->AbsoluteMS(); for (int i = 0; i < st_count; ++i) { sig3.Emit(10, 20, 30); }
        functionBindSignalResultString += Format(" dava signal 3: %llu\n", DAVA::SystemTimer::Instance()->AbsoluteMS() - time_ms);
#endif
    }

    DAVA_TEST(TestFunctionExtended)
    {
        B b; A* a_pt = nullptr;
        void *void_test = nullptr;
        const char *char_test = nullptr;

        Function<void* (void *)> sta0(&exStaticFnV);
        Function<void* (void **)> sta1(&exStaticFnVV);
        Function<const char* (const char *)> sta2(&exStaticFnCC);
        Function<const char* (const char **)> sta3(&exStaticFnCCC);

        TEST_VERIFY(sta0(void_test) == exStaticFnV(void_test));
        TEST_VERIFY(sta1(&void_test) == exStaticFnVV(&void_test));
        TEST_VERIFY(sta2(char_test) == exStaticFnCC(char_test));
        TEST_VERIFY(sta3(&char_test) == exStaticFnCCC(&char_test));

        Function<A* (B*, A*)> cla0(&B::exClassFn);
        Function<const A* (B*, const A*)> cla1(&B::exClassFn1);
        Function<A* (B*, A**)> cla2(&B::exClassFn2);
        Function<const A* (B*, const A**)> cla3(&B::exClassFn3);

        TEST_VERIFY(cla0(&b, a_pt) == b.exClassFn(a_pt));
        TEST_VERIFY(cla1(&b, a_pt) == b.exClassFn1(a_pt));
        TEST_VERIFY(cla2(&b, &a_pt) == b.exClassFn2(&a_pt));
        TEST_VERIFY(cla3(&b, (const A**)&a_pt) == b.exClassFn3((const A**)&a_pt));
    }
};
