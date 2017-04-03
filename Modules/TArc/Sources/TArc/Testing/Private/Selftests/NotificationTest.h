#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Core/OperationRegistrator.h"

#include "TArc/WindowSubSystem/UI.h"

#include "TArc/Controls/Noitifications/NotificationLayout.h"
#include "TArc/Controls/Noitifications/NotificationWidget.h"

#include "TArc/Utils/QtConnections.h"

#include <QPushButton>

namespace NotificationTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("NotificationsTestWnd");
DECLARE_OPERATION_ID(ShowNotificationOperation);
IMPL_OPERATION_ID(ShowNotificationOperation);

class NotificationTestModule : public DAVA::TArc::ClientModule
{
public:
    NotificationTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;

        QWidget* w = new QWidget();

        DAVA::TArc::PanelKey panelKey("NotificationTest", DAVA::TArc::CentralPanelInfo());
        GetUI()->AddView(wndKey, panelKey, w);

        RegisterOperation(ShowNotificationOperation.ID, this, &NotificationTestModule::ShowNotification);
    }

    void ShowNotification(const DAVA::String& title, const DAVA::Result& message, DAVA::Function<void(void)> callback)
    {
        NotificationParams params;
        params.message = message;
        params.title = title;
        params.callback = callback;
        GetUI()->ShowNotification(wndKey, params);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(NotificationTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<NotificationTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
    static NotificationTestModule* instance;
};

NotificationTestModule* NotificationTestModule::instance = nullptr;
}

DAVA_TARC_TESTCLASS(NotificationTest)
{
    DAVA_TEST (ShowNotificationAndClose)
    {
        using namespace DAVA;
        using namespace testing;

        EXPECT_CALL(*this, OnDestroyed());

        Result result(Result::RESULT_SUCCESS, "test str");
        Function<void()> callBack;
        InvokeOperation(NotificationTestDetails::ShowNotificationOperation.ID, String("sample title"), result, callBack);
        ClickCloseButton();
    }

    DAVA_TEST (ShowOneNotification)
    {
        using namespace DAVA;
        using namespace testing;

        Result result(Result::RESULT_SUCCESS, "test str");
        EXPECT_CALL(*this, Callback());
        EXPECT_CALL(*this, OnDestroyed());

        InvokeOperation(NotificationTestDetails::ShowNotificationOperation.ID, String("sample title"), result, MakeFunction(this, &NotificationTest::Callback));
        ClickDetailsButton();
    }

    DAVA_TEST (TestNotificationTime)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;
        using namespace testing;

        QElapsedTimer elapsedTimer;
        elapsedTimer.start();

        const int timeout = 100;
        EXPECT_CALL(*this, OnDestroyed())
        .WillOnce(Invoke([&elapsedTimer, timeout]() {
            int elapsed = elapsedTimer.elapsed();
            TEST_VERIFY(elapsed == timeout);
        }));

        QWidget* parent = GetWindow(NotificationTestDetails::wndKey);
        NotificationLayout notificationLayout;
        notificationLayout.SetDisplayTimeMs(timeout);

        NotificationWidgetParams params;
        notificationLayout.AddNotificationWidget(parent, params);
        NotificationWidget* child = parent->findChild<NotificationWidget*>();
        connections.AddConnection(child, &QObject::destroyed, MakeFunction(this, &NotificationTest::OnDestroyed));
    }

    void ClickCloseButton()
    {
        QWidget* parent = GetWindow(NotificationTestDetails::wndKey);
        QPushButton* button = parent->findChild<QPushButton*>(QStringLiteral("CloseButton"));
        connections.AddConnection(button, &QObject::destroyed, DAVA::MakeFunction(this, &NotificationTest::OnDestroyed));
        TEST_VERIFY(button != nullptr);
        button->click();
    }

    void ClickDetailsButton()
    {
        QWidget* parent = GetWindow(NotificationTestDetails::wndKey);
        QPushButton* button = parent->findChild<QPushButton*>(QStringLiteral("DetailsButton"));
        connections.AddConnection(button, &QObject::destroyed, DAVA::MakeFunction(this, &NotificationTest::OnDestroyed));
        TEST_VERIFY(button != nullptr);
        button->click();
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());
    MOCK_METHOD0_VIRTUAL(Callback, void());
    MOCK_METHOD0_VIRTUAL(OnDestroyed, void());
    MOCK_METHOD0_VIRTUAL(OnTimeout, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(NotificationTestDetails::NotificationTestModule);
    END_TESTED_MODULES()

    QtConnections connections;
};
