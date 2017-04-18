#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Core/OperationRegistrator.h"

#include "TArc/WindowSubSystem/UI.h"

#include "TArc/Controls/Private/NotificationLayout.h"
#include "TArc/Controls/Private/NotificationWidget.h"

#include "TArc/Utils/QtConnections.h"

#include <Base/BaseTypes.h>

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
        w->resize(800, 600);

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

        const int timeout = 50;

        NotificationLayout* layout = new NotificationLayout();
        layout->SetDisplayTimeMs(timeout);

        QWidget* parent = GetWindow(NotificationTestDetails::wndKey);
        NotificationParams params;
        QElapsedTimer* elapsedTimer = new QElapsedTimer;
        elapsedTimer->start();

        layout->ShowNotification(parent, params);

        EXPECT_CALL(*this, OnDestroyed())
        .WillOnce(Invoke([layout, elapsedTimer, timeout]() {
            int elapsedMs = elapsedTimer->elapsed();
            const int maxExpectedTimeMs = 300; //time can be increased by animations or NotificationLayout timer accuracy
            TEST_VERIFY(elapsedMs < maxExpectedTimeMs);
            //layout iterate widgets and can not be deleted right now
            layout->deleteLater();
            delete elapsedTimer;
        }));

        QList<NotificationWidget*> widgetList = parent->findChildren<NotificationWidget*>();
        NotificationWidget* firstNotification = widgetList.first();
        connections.AddConnection(firstNotification, &QObject::destroyed, MakeFunction(this, &NotificationTest::OnDestroyed));
    }

    DAVA_TEST (TestNotificationsCount)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;
        using namespace testing;

        NotificationParams params;

        NotificationLayout layout;
        QWidget* parent = GetWindow(NotificationTestDetails::wndKey);
        for (int i = 0; i < 20; ++i)
        {
            layout.ShowNotification(parent, params);
        }
        QList<NotificationWidget*> widgetList = parent->findChildren<NotificationWidget*>();
        NotificationWidget* firstNotification = widgetList.first();
        NotificationWidget* lastNotification = widgetList.last();
        TEST_VERIFY(firstNotification->isVisible() == true);
        TEST_VERIFY(lastNotification->isVisible() == false);
    }

    DAVA_TEST (TestNotificationsLayout)
    {
        using namespace DAVA;
        using namespace DAVA::TArc;
        using namespace testing;

        QWidget* parent = GetWindow(NotificationTestDetails::wndKey);

        using PositionFn = Function<QPoint(QWidget * parent, QWidget * bubble)>;
        using PositionFnWithAlign = std::pair<uint64, PositionFn>;
        Vector<PositionFnWithAlign> positions = {
            { ALIGN_TOP | ALIGN_LEFT, [](QWidget* parent, QWidget* bubble) { return QPoint(0, 0); } },
            { ALIGN_TOP | ALIGN_RIGHT, [](QWidget* parent, QWidget* bubble) { return QPoint(parent->width() - bubble->width(), 0); } },
            { ALIGN_BOTTOM | ALIGN_LEFT, [](QWidget* parent, QWidget* bubble) { return QPoint(0, parent->height() - bubble->height()); } },
            { ALIGN_BOTTOM | ALIGN_RIGHT, [](QWidget* parent, QWidget* bubble) { return QPoint(parent->width() - bubble->width(), parent->height() - bubble->height()); } }
        };

        const int positionsCount = 4;
        for (int i = 0; i < positionsCount; ++i)
        {
            NotificationLayout layout;

            const PositionFnWithAlign& alignWithFn = positions.at(i);

            layout.SetLayoutType(alignWithFn.first);

            NotificationParams params;
            layout.ShowNotification(parent, params);

            QList<NotificationWidget*> widgetList = parent->findChildren<NotificationWidget*>();
            NotificationWidget* first = widgetList.first();

            QPoint firstPos(first->pos());
            PositionFn positionFn = alignWithFn.second;
            QPoint parentPos = parent->mapToGlobal(positionFn(parent, first));
            TEST_VERIFY(firstPos == parentPos);
        }
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

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(NotificationTestDetails::NotificationTestModule);
    END_TESTED_MODULES()

    QtConnections connections;
};
