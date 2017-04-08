#include "Tests/AssertTest.h"

#include <Debug/MessageBox.h>
#include <Engine/Engine.h>
#include <Job/JobManager.h>
#include <Logger/Logger.h>
#include <UI/Input/UIActionBindingComponent.h>
#include <UI/Input/UIActionBindingComponent.h>
#include <UI/Update/UIUpdateComponent.h>

const static DAVA::float32 DEFAULT_TIMEOUT = 3.f;

AssertTest::AssertTest(TestBed& app)
    : BaseScreen(app, "AssertTest")
{
    GetOrCreateComponent<DAVA::UIUpdateComponent>();
}

void AssertTest::LoadResources()
{
    BaseScreen::LoadResources();

    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/UI/AssertTestScreen.yaml", &pkgBuilder);
    DAVA::UIControl* dialog = pkgBuilder.GetPackage()->GetControl("MainFrame");
    dialog->SetSize(DAVA::Vector2(500, 500));
    AddControl(dialog);

    DAVA::UIActionBindingComponent* actions = dialog->GetComponent<DAVA::UIActionBindingComponent>();
    if (actions)
    {
        actions->GetActionMap().Put(DAVA::FastName("ASSERT_ALWAYS"), [&]() {
            DVASSERT_ALWAYS(false, "Demo assert");
        });
        actions->GetActionMap().Put(DAVA::FastName("ASSERT"), [&]() {
            DVASSERT(false, "Demo assert");
        });
        actions->GetActionMap().Put(DAVA::FastName("DELAYED_ASSERT"), [&]() {
            timeOut = DEFAULT_TIMEOUT;
        });

        actions->GetActionMap().Put(DAVA::FastName("MSGBOX_MAINTHREAD"), DAVA::MakeFunction(this, &AssertTest::ShowMessageBoxFromMainThread));
        actions->GetActionMap().Put(DAVA::FastName("MSGBOX_UITHREAD"), DAVA::MakeFunction(this, &AssertTest::ShowMessageBoxFromUIThread));
        actions->GetActionMap().Put(DAVA::FastName("MSGBOX_OTHERTHREADS"), DAVA::MakeFunction(this, &AssertTest::ShowMessageBoxFromOtherThreads));
    }

    countdownText = static_cast<DAVA::UIStaticText*>(dialog->FindByName("Countdown"));
}

void AssertTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    countdownText.Set(nullptr);
}

void AssertTest::Update(DAVA::float32 timeElapsed)
{
    BaseScreen::Update(timeElapsed);
    if (timeOut > 0.f)
    {
        timeOut -= timeElapsed;
        if (timeOut <= 0.f)
        {
            timeOut = 0.f;
            DVASSERT(false, "Demo assert");
        }
    }
    if (countdownText.Get())
    {
        countdownText->SetText(DAVA::Format(L"%1.1f", timeOut));
    }
}

void AssertTest::ShowMessageBoxFromMainThread()
{
    using namespace DAVA;
    int r = Debug::MessageBox("Message box", "Message box from main thread", { "Ping", "Pong", "Kaboom" }, 1);
    Logger::Debug("You choose button %d", r);
}

void AssertTest::ShowMessageBoxFromUIThread()
{
    using namespace DAVA;
    Window* primaryWindow = GetPrimaryWindow();
    primaryWindow->RunOnUIThreadAsync([]() {
        int r = Debug::MessageBox("Message box", "Message box from UI thread", { "Uno", "Duo", "Trio" }, 2);
        Logger::Debug("You choose button %d", r);
    });
}

void AssertTest::ShowMessageBoxFromOtherThreads()
{
    using namespace DAVA;
    JobManager* jobman = GetEngineContext()->jobManager;
    for (int i = 0; i < 3; ++i)
    {
        jobman->CreateWorkerJob([i]() {
            uint64 threadId = Thread::GetCurrentIdAsUInt64();
            String msg = Format("Message box #%d from thread 0x%llX", i, threadId);
            Vector<String> buttons;
            switch (i)
            {
            case 0:
                buttons = { "Click me" };
                break;
            case 1:
                buttons = { "To be", "Not to be" };
                break;
            case 2:
                buttons = { "Error", "Warning", "Info" };
                break;
            }
            int r = Debug::MessageBox("Message box", msg, buttons, i);
            Logger::Debug("You choose button %d [#%d thread=0x%llx]", r, i, threadId);
        });
    }
}
