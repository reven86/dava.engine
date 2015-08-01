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

#include <functional>
#include <time.h>
#include "Tests/FunctionSignalTest.h"

using namespace DAVA;

template<typename F, typename... A>
String BenchTest(const char* name, F& f, A... a)
{
    int st_count = 99999999;
    int res = 0;

    clock_t time_ms = clock();
    for (int i = 0; i < st_count; ++i)
    {
        res += f(std::forward<A>(a)..., i);
    }

    return Format("%s: %lu ms, res = %d\n", name, clock() - time_ms, res);
}

#define BENCH_TEST(name, fn, ...) BenchTest(name, fn, ##__VA_ARGS__)

FunctionSignalTest::FunctionSignalTest()
    : BaseScreen("FunctionSignalTest")
{ }

int test(int a, int b, int i)
{
    return (i * (a + b));
}

struct TestStruct {
    int test(int a, int b, int i)
    {
        return (i * (a + b));
    }
};

void FunctionSignalTest::LoadResources()
{
    BaseScreen::LoadResources();

    Font *font30 = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font30);
    font30->SetSize(30);

    Font *font12 = FTFont::Create("~res:/Fonts/DroidSansMono.ttf");
    DVASSERT(font12);
    font12->SetSize(14);

    runResult = new UIStaticText(Rect(10, 10, 450, 600));
    runResult->SetFont(font12);
    runResult->SetTextColor(Color::White);
    runResult->SetDebugDraw(true);
    runResult->SetMultiline(true);
    runResult->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(runResult);

    runButton = new UIButton(Rect(10, 620, 450, 60));
    runButton->SetStateFont(0xFF, font30);
    runButton->SetStateFontColor(0xFF, Color::White);
    runButton->SetStateText(0xFF, L"Start bench test");
    runButton->SetStateText(UIButton::STATE_DISABLED, L"Running...");
    runButton->SetDisabled(false);
    runButton->SetDebugDraw(true);
    runButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &FunctionSignalTest::OnButtonPress));
    AddControl(runButton);
}

void FunctionSignalTest::UnloadResources()
{
    SafeRelease(runResult);
    SafeRelease(runButton);
}

DAVA_NOINLINE int viewDavaFnCall(Function<int(int, int, int)> &fn)
{
    return fn(10, 20, 30);
}

void DoFunctionSignalTest(FunctionSignalTest *fst)
{
    TestStruct ts;
    
    std::function<int(int, int, int)> stdStatic(&test);
    Function<int(int, int, int)> davaStatic(&test);
    
    int rrr = viewDavaFnCall(davaStatic);
    printf("%d\n", rrr);
    
    JobManager::Instance()->CreateMainJob([fst]{
        fst->runResult->SetText(L"Started...");
    });
    
    String resStr = "Static function:\n";
    resStr += BENCH_TEST("  native", test, 10, 20);
    resStr += BENCH_TEST("  std   ", stdStatic, 10, 20);
    resStr += BENCH_TEST("  dava  ", davaStatic, 10, 20);
    
    JobManager::Instance()->CreateMainJob([fst, resStr]{
        fst->runResult->SetText(StringToWString(resStr));
    });
    
    int cap1 = 10, cap2 = 20;
    auto lam = [&cap1, &cap2](int index) -> int { return cap1 + cap2 * index; };
    std::function<int(int)> stdLam(lam);
    Function<int(int)> davaLam(lam);
    
    resStr += "\nLambda function:\n";
    resStr += BENCH_TEST("  native", lam);
    resStr += BENCH_TEST("  std   ", stdLam);
    resStr += BENCH_TEST("  dava  ", davaLam);
    
    JobManager::Instance()->CreateMainJob([fst, resStr]{
        fst->runResult->SetText(StringToWString(resStr));
    });

    auto nativeCls = [](TestStruct *ts, int a, int b, int i) -> int { return ts->test(a, b, i); };
    std::function<int(TestStruct *ts, int, int, int)> stdCls(std::mem_fn(&TestStruct::test));
    Function<int(TestStruct *ts, int, int, int)> davaCls(&TestStruct::test);
    
    resStr += "\nClass function:\n";
    resStr += BENCH_TEST("  native", nativeCls, &ts, 10, 20);
    resStr += BENCH_TEST("  std   ", stdCls, &ts, 10, 20);
    resStr += BENCH_TEST("  dava  ", davaCls, &ts, 10, 20);
    
    JobManager::Instance()->CreateMainJob([fst, resStr]{
        fst->runResult->SetText(StringToWString(resStr));
    });

    auto nativeObj = [&ts](int a, int b, int i) -> int { return ts.test(a, b, i); };
    std::function<int(int, int, int)> stdObj(std::bind(&TestStruct::test, &ts, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Function<int(int, int, int)> davaObj(&ts, &TestStruct::test);
    
    resStr += "\nObj function:\n";
    resStr += BENCH_TEST("  native", nativeObj, 10, 20);
    resStr += BENCH_TEST("  std   ", stdObj, 10, 20);
    resStr += BENCH_TEST("  dava  ", davaObj, 10, 20);
    
    JobManager::Instance()->CreateMainJob([fst, resStr]{
        fst->runResult->SetText(StringToWString(resStr));
    });

    auto nativeBind = std::bind(&TestStruct::test, &ts, 10, 20, std::placeholders::_1);
    std::function<int(int)> stdBind = std::bind(&TestStruct::test, &ts, 10, 20, std::placeholders::_1);
    Function<int(int)> davaBind = std::bind(&TestStruct::test, &ts, 10, 20, std::placeholders::_1);
    
    resStr += "\nBinded function:\n";
    resStr += BENCH_TEST("  native", nativeBind);
    resStr += BENCH_TEST("  std   ", stdBind);
    resStr += BENCH_TEST("  dava  ", davaBind);
    
    resStr += "\n\nDone!";
    
    JobManager::Instance()->CreateMainJob([fst, resStr]{
        fst->runResult->SetText(StringToWString(resStr));
        fst->runButton->SetDisabled(false);
    });
}

void FunctionSignalTest::OnButtonPress(BaseObject *obj, void *data, void *callerData)
{
    if(!runButton->GetDisabled())
    {
        runButton->SetDisabled(true);
        JobManager::Instance()->CreateWorkerJob(std::bind(&DoFunctionSignalTest, this));

    }
}
