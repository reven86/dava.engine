#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/ReflectedButton.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"

#include <TArc/Qt/QtString.h>
#include <TArc/Qt/QtIcon.h>
#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Functional/Function.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QtTest>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>

namespace ReflectedButtonTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("ReflectedButtonTestWnd");

struct ReflectedButtonDataSource
{
    DAVA::int32 value = 0;
    bool enabled = false;
    bool autoRaise = false;

    DAVA::int32 GetValue() const
    {
        return value;
    }

    void SetValue(DAVA::int32 v)
    {
        value = v;
    }

    DAVA::int32 GetNewValue() const
    {
        return value + 3;
    }

    QString GetText() const
    {
        return "Text";
    }

    QIcon GetIcon() const
    {
        return SharedIcon(":/QtIcons/remove.png");
    }

    DAVA_REFLECTION(ReflectedButtonDataSource)
    {
        DAVA::ReflectionRegistrator<ReflectedButtonDataSource>::Begin()
        .Method("newValue", &ReflectedButtonDataSource::GetNewValue)
        .Field("value", &ReflectedButtonDataSource::GetValue, &ReflectedButtonDataSource::SetValue)
        .Field("icon", &ReflectedButtonDataSource::GetIcon, nullptr)
        .Field("text", &ReflectedButtonDataSource::GetText, nullptr)
        .Field("enabled", &ReflectedButtonDataSource::enabled)
        .Field("autoRaise", &ReflectedButtonDataSource::autoRaise)
        .End();
    }
};

class ReflectedButtonTestModule : public DAVA::TArc::ClientModule
{
public:
    ReflectedButtonTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        DAVA::Reflection reflectedModel = DAVA::Reflection::Create(&dataSource);

        {
            ReflectedButton::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ReflectedButton::Fields::Clicked] = "newValue";
            params.fields[ReflectedButton::Fields::Icon] = "icon";
            params.fields[ReflectedButton::Fields::Text] = "text";
            params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
            params.fields[ReflectedButton::Fields::Result] = "value";

            ReflectedButton* button = new ReflectedButton(params, GetAccessor(), reflectedModel);
            button->SetObjectName("ReflectedButton_enabled");
            layout->AddControl(button);
        }

        {
            ReflectedButton::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ReflectedButton::Fields::Clicked] = "newValue";
            params.fields[ReflectedButton::Fields::Icon] = "icon";
            params.fields[ReflectedButton::Fields::Text] = "text";
            params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
            params.fields[ReflectedButton::Fields::Result] = "value";
            params.fields[ReflectedButton::Fields::Enabled] = "enabled";

            ReflectedButton* button = new ReflectedButton(params, GetAccessor(), reflectedModel);
            button->SetObjectName("ReflectedButton_disabled");
            layout->AddControl(button);
        }

        GetUI()->AddView(wndKey, DAVA::TArc::PanelKey("ReflectedButtonSandbox", DAVA::TArc::CentralPanelInfo()), w);
    }

    ReflectedButtonDataSource dataSource;

    static ReflectedButtonTestModule* instance;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ReflectedButtonTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<ReflectedButtonTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

ReflectedButtonTestModule* ReflectedButtonTestModule::instance = nullptr;
}

DAVA_TARC_TESTCLASS(ReflectedButtonTest)
{
    QToolButton* GetButton(const QString& name)
    {
        QList<QWidget*> widgets = LookupWidget(ReflectedButtonTestDetails::wndKey, name);
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QToolButton* button = qobject_cast<QToolButton*>(w);
        TEST_VERIFY(button != nullptr);
        return button;
    }

    DAVA_TEST (RBEnabledTest)
    {
        using namespace DAVA;
        using namespace ReflectedButtonTestDetails;

        ReflectedButtonTestModule* inst = ReflectedButtonTestModule::instance;
        int32 oldValue = inst->dataSource.value;

        QToolButton* button = GetButton("ReflectedButton_enabled");
        TEST_VERIFY(Any(button->icon()) == Any(inst->dataSource.GetIcon()));
        TEST_VERIFY(button->text() == inst->dataSource.GetText());
        TEST_VERIFY(button->autoRaise() == inst->dataSource.autoRaise);

        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers());
        eventList.simulate(button);

        TEST_VERIFY(oldValue + 3 == inst->dataSource.value);
    }

    DAVA_TEST (RBDisabledTest)
    {
        using namespace DAVA;
        using namespace ReflectedButtonTestDetails;

        ReflectedButtonTestModule* inst = ReflectedButtonTestModule::instance;
        int32 oldValue = inst->dataSource.value;

        QToolButton* button = GetButton("ReflectedButton_disabled");
        TEST_VERIFY(Any(button->icon()) == Any(inst->dataSource.GetIcon()));
        TEST_VERIFY(button->text() == inst->dataSource.GetText());
        TEST_VERIFY(button->autoRaise() == inst->dataSource.autoRaise);

        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers());
        eventList.simulate(button);

        TEST_VERIFY(oldValue == inst->dataSource.value);
    }

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ReflectedButtonTestDetails::ReflectedButtonTestModule);
    END_TESTED_MODULES()
};
