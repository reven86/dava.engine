#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QtTest>

namespace LineEditTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("LineEditTestWnd");

class LineEditTestModule : public DAVA::TArc::ClientModule
{
public:
    LineEditTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        model.emplace("text", DAVA::String("Line edit text"));
        DAVA::TArc::LineEdit::FieldsDescriptor descr;
        descr.valueFieldName = DAVA::FastName("text");
        DAVA::TArc::LineEdit* edit = new DAVA::TArc::LineEdit(descr, GetAccessor(), DAVA::Reflection::Create(&model));

        DAVA::TArc::PanelKey key("LineEdit", DAVA::TArc::CentralPanelInfo());
        GetUI()->AddView(wndKey, key, edit->ToWidgetCast());
    }

    DAVA::Map<DAVA::String, DAVA::Any> model;

    static LineEditTestModule* instance;

    DAVA_VIRTUAL_REFLECTION(LineEditTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<LineEditTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

LineEditTestModule* LineEditTestModule::instance = nullptr;
}

DAVA_TARC_TESTCLASS(LineEditTest)
{
    DAVA_TEST (EditTextTest)
    {
        using namespace LineEditTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("LineEdit"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint(1, 0));
        for (int i = 0; i < 5; ++i)
        {
            eventList.addKeyClick(Qt::Key_Delete);
        }

        eventList.addKeyClicks(QString("Reflected line "));
        eventList.addKeyClick(Qt::Key_Enter);
        eventList.simulate(w);

        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(w);
        TEST_VERIFY(lineEdit != nullptr);
        TEST_VERIFY(lineEdit->text() == QString("Reflected line edit text"));
        LineEditTestModule* inst = LineEditTestModule::instance;
        TEST_VERIFY(inst->model.find("text") != inst->model.end());
        TEST_VERIFY(inst->model["text"].Cast<DAVA::String>() == DAVA::String("Reflected line edit text"));
    }

    DAVA_TEST (EditModelTextTest)
    {
        using namespace LineEditTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("LineEdit"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(w);
        connections.AddConnection(lineEdit, &QLineEdit::textChanged, DAVA::MakeFunction(this, &LineEditTest::OnTextChanged));

        EXPECT_CALL(*this, OnTextChanged(QString("Fully Changed text")))
        .WillOnce(Return());

        LineEditTestModule::instance->model["text"] = DAVA::String("Fully Changed text");
    }

    MOCK_METHOD1_VIRTUAL(OnTextChanged, void(const QString& s));

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(LineEditTestDetails::LineEditTestModule);
    END_TESTED_MODULES()

    DAVA::TArc::QtConnections connections;
};
