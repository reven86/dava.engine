#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#include "Infrastructure/TextureUtils.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Platform/TemplateWin32/DispatcherWinUAP.h"

using namespace DAVA;

DAVA_TESTCLASS (DispatcherWinUAPTest)
{
    DispatcherWinUAPTest()
        : dispatcher1(std::make_unique<DispatcherWinUAP>())
        , dispatcher2(std::make_unique<DispatcherWinUAP>())
    {
        int h = 0;
    }

    DAVA_TEST (DispatcherTest)
    {
        ScopedPtr<Thread> thread1(Thread::Create([this]() { Thread1(); }));
        ScopedPtr<Thread> thread2(Thread::Create([this]() { Thread2(); }));

        thread1->Start();
        thread2->Start();

        thread1->Join();
        thread2->Join();

        TEST_VERIFY(counter == 10);
        TEST_VERIFY(stage1 == 3);
        TEST_VERIFY(stage2 == 1);
    }

    void Thread1()
    {
        dispatcher1->BindToCurrentThread();
        while (!byeThread1)
        {
            dispatcher1->ProcessTasks();
            switch (stage1)
            {
            case 0:
                // Run increment on other thread
                for (int32 i = 0; i < 10; ++i)
                {
                    dispatcher2->RunAsync([this]() { counter += 1; });
                }
                stage1 += 1;
                break;
            case 1:
                // Check whether task is executed in the second dispatcher context
                dispatcher2->RunAsync([this]() {
                    TEST_VERIFY(dispatcher2->BoundThreadId() == Thread::GetCurrentId());
                });
                stage1 += 1;
                break;
            case 2:
                // Test blocking call
                dispatcher2->RunAsyncAndWait([this]() {
                    checkMe = 42;
                });
                TEST_VERIFY(checkMe == 42);
                stage1 += 1;
                break;
            case 3:
                // Test completion task
                dispatcher2->RunAsync([this]() {
                    byeThread1 = true;
                    byeThread2 = true;
                });
                break;
            }
        }
    }

    void Thread2()
    {
        dispatcher2->BindToCurrentThread();
        while (!byeThread2)
        {
            dispatcher2->ProcessTasks();
            switch (stage2)
            {
            case 0:
                dispatcher1->RunAsyncAndWait([this]() {
                    TEST_VERIFY(dispatcher1->BoundThreadId() == Thread::GetCurrentId());
                });
                stage2 += 1;
                break;
            }
        }
    }

    std::unique_ptr<DispatcherWinUAP> dispatcher1;
    std::unique_ptr<DispatcherWinUAP> dispatcher2;
    volatile bool byeThread1 = false;
    volatile bool byeThread2 = false;

    int32 stage1 = 0;
    int32 stage2 = 0;
    int32 counter = 0;
    int32 checkMe = -1;
};

#endif
