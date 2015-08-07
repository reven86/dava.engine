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
#include "Functional/Signal.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

// =======================================================================================================================================
// =======================================================================================================================================
// =======================================================================================================================================

static String functionBindSignalResultString;

int staticFn0() { return 100; }
int staticFn3(int i1, int i2, int i3) { return i1 + i2 + i3; }

void* exStaticFnV(void *a) { return a; }
void* exStaticFnVV(void **a) { return *a; }
const char* exStaticFnCC(const char *a) { return a; }
const char* exStaticFnCCC(const char **a) { return *a; }

struct A
{
    int classFn0() { return 100; }
    int classFn3(int i1, int i2, int i3) { return i1 + i2 + i3; }

    int classFn0_const() const { return 100; }
    int classFn3_const(int i1, int i2, int i3) const { return i1 + i2 + i3; }

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

    int getV() { return v*v; }
};

struct M
{
    char c_[128];
    int getV() { return 10; }
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
        Function<int(int, int, int)> static_f3(&staticFn3);

        TEST_VERIFY(static_f0() == staticFn0());
        TEST_VERIFY(static_f3(3, 8, 5) == staticFn3(3, 8, 5));

        A a;
        Function<int(A*)> class_f0 = &A::classFn0;
        Function<int(A*, int, int, int)> class_f3 = &A::classFn3;

        Function<int(A*)> class_f0c = &A::classFn0_const;
        Function<int(A*, int, int, int)> class_f3c = &A::classFn3_const;

        TEST_VERIFY(class_f0(&a) == a.classFn0());
        TEST_VERIFY(class_f3(&a, 3, 8, 5) == a.classFn3(3, 8, 5));

        TEST_VERIFY(class_f0c(&a) == a.classFn0_const());
        TEST_VERIFY(class_f3c(&a, 3, 8, 5) == a.classFn3_const(3, 8, 5));

        // ==================================================================================
        // function with assigned object
        // ==================================================================================
        Function<int()> object_f0(&a, &A::classFn0);
        Function<int(int, int, int)> object_f3(&a, &A::classFn3);

        Function<int()> object_f0c(&a, &A::classFn0_const);
        Function<int(int, int, int)> object_f3c(&a, &A::classFn3_const);

        TEST_VERIFY(object_f0() == a.classFn0());
        TEST_VERIFY(object_f3(3, 8, 5) == a.classFn3(3, 8, 5));

        TEST_VERIFY(object_f0c() == a.classFn0_const());
        TEST_VERIFY(object_f3c(3, 8, 5) == a.classFn3_const(3, 8, 5));

        // ==================================================================================
        // MakeFunction helper testing
        // ==================================================================================
        MakeFunction(&staticFn0)();
        MakeFunction(&staticFn3)(1, 2, 3);

        MakeFunction(&A::classFn0)(&a);
        MakeFunction(&A::classFn3)(&a, 1, 2, 3);

        MakeFunction(static_f3)(1, 2, 3);
        MakeFunction(class_f3)(&a, 1, 2, 3);

        // ==================================================================================
        // inherited class testing
        // ==================================================================================
        B b;
        Function<void(B*, int)> b0_class_setV = &B::setV;
        Function<void(A*, int)> b1_class_setV = &B::setV;

        Function<int(A*)> b3_class_getV = &A::getV;
        Function<int(B*)> b4_class_getV = &B::getV;
        Function<int(B*)> b5_class_getV = &A::getV;

        b0_class_setV(&b, 100); TEST_VERIFY(b.getV() == (100 * 100));
        b1_class_setV(&b, 200); TEST_VERIFY(b.getV() == (200 * 200));
        b1_class_setV(&a, 100); TEST_VERIFY(a.getV() == 100);
        b.setV(300); TEST_VERIFY(b3_class_getV(&b) == 300);
        a.setV(300); TEST_VERIFY(b3_class_getV(&a) == 300);
        b.setV(400); TEST_VERIFY(b4_class_getV(&b) == (400 * 400));
        b.setV(500); TEST_VERIFY(b5_class_getV(&b) == 500);

        Function<void(int)> b0_obj_setV(&b, &B::setV);
        Function<void(int)> b1_obj_setV(&b, &A::setV);
        Function<int()> b3_obj_getV(&b, &A::getV);
        Function<int()> b4_obj_getV(&b, &B::getV);

        Function<void(B*, int)> a0_class_setV = &A::setV;
        Function<void(A*, int)> a1_class_setV = &A::setV;
        Function<void(int)> a0_obj_setV(&a, &B::setV);
        Function<void(int)> a1_obj_setV(&a, &A::setV);
        Function<int()> a3_obj_getV(&a, &A::getV);

#if 0
        // thous functions should assert during compilation
        Function<int(B*)> no_b0_class_getV1 = &M::getV;
        Function<int()> no_a0_obj_getV(&a, &B::getV);
#endif

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
        Function<int()> null_f0_1 = nullptr;

        TEST_VERIFY(null_f0 == null_f0_1);
        TEST_VERIFY(null_f0 == nullptr);
        TEST_VERIFY(null_f0_1 == nullptr);

        null_f0 = static_f0;
        TEST_VERIFY(null_f0() == staticFn0());
        null_f0 = 0;
        TEST_VERIFY(null_f0 == nullptr);

        null_f0 = object_f0;
        TEST_VERIFY(null_f0() == object_f0());

        // ==================================================================================
        // bind testing
        // ==================================================================================
        A aa;
        Function<int()> bound_create_f0 = std::bind(&A::classFn0, &aa);
        TEST_VERIFY(bound_create_f0() == a.classFn0());

        Function<int()> bound_f0 = std::bind(class_f0, &aa);
        Function<int(A*)> bound_f0_1 = std::bind(class_f0, std::placeholders::_1);

        bound_f0();

        TEST_VERIFY(bound_f0() == class_f0(&aa));
        TEST_VERIFY(bound_f0_1(&aa) == class_f0(&aa));

        Function<int()> bound_f3 = std::bind(&A::classFn3, &aa, 10, 20, 30);
        Function<int(int)> bound_f3_1 = std::bind(&A::classFn3, &aa, std::placeholders::_1, 20, 30);
        Function<int(A*)> bound_f3_2 = std::bind(&A::classFn3, std::placeholders::_1, 10, 20, 30);
        Function<int(int, int, int, A*)> bound_f3_3 = std::bind(&A::classFn3, std::placeholders::_4, std::placeholders::_3, std::placeholders::_2, std::placeholders::_1);

        TEST_VERIFY(bound_f3() == aa.classFn3(10, 20, 30));
        TEST_VERIFY(bound_f3_1(10) == aa.classFn3(10, 20, 30));
        TEST_VERIFY(bound_f3_2(&aa) == aa.classFn3(10, 20, 30));
        TEST_VERIFY(bound_f3_3(30, 20, 10, &aa) == aa.classFn3(10, 20, 30));
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


    class sgA : public TrackedObject
    {
    public:
        int a = 0;
        void AddA() { a++; }
    };

    class sgB
    {
    public:
        virtual ~sgB() { };
        int b;
    };

    class sgC : public sgB, public TrackedObject
    {
    public:
        int c;
        void AddC() { c++; }
    };

    DAVA_TEST(TestSignals)
    {
        // ==================================================================================
        // signals
        // ==================================================================================
        Signal<> sig0;

        // track object deletion, while it is tracked by signal
        sgA *a1 = new sgA();
        sig0.Track(a1, sig0.Connect([&a1]{ a1->AddA(); }));
        sig0.Connect(a1, &sgA::AddA);
        sig0.Emit();
        delete a1;
        sig0.Emit(); // <-- this shouldn't crash

        // track signal deletion, while tracking object exists
        sgC *c1 = new sgC();
        Signal<> *sig1 = new Signal<>();
        sig1->Connect(c1, &sgC::AddC);
        delete sig1;
        delete c1; // <-- this shouldn't crash
    }
};
