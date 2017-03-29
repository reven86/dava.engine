#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/PlainTextEdit.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QPlainTextEdit>
#include <QtTest>
#include <QEvent>

namespace PlainTextEditTestDetails
{
using namespace DAVA;

DAVA::TArc::WindowKey wndKey = DAVA::FastName("PlainTextEditTestWnd");

struct PlainTextEditDataSource
{
    String text = "";
    String placeHolder = "placeHolder";

    bool isReadOnly = false;
    bool isEnabled = true;

    const String& GetText() const
    {
        return text;
    }

    DAVA_REFLECTION(PlainTextEditDataSource)
    {
        ReflectionRegistrator<PlainTextEditDataSource>::Begin()
        .Field("readOnlyTextMeta", &PlainTextEditDataSource::text)[M::ReadOnly()]
        .Field("readOnlyText", &PlainTextEditDataSource::GetText, nullptr)
        .Field("text", &PlainTextEditDataSource::text)
        .Field("shortText", &PlainTextEditDataSource::text)[M::MaxLength(5)]
        .Field("placeHolder", &PlainTextEditDataSource::placeHolder)
        .Field("isReadOnly", &PlainTextEditDataSource::isReadOnly)
        .Field("isEnabled", &PlainTextEditDataSource::isEnabled)
        .End();
    }
};

class PlainTextEditTestModule : public DAVA::TArc::ClientModule
{
public:
    PlainTextEditTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;
        model.emplace("text", DAVA::String("Plain text edit text"));

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        {
            ControlDescriptorBuilder<PlainTextEdit::Fields> descr;
            descr[PlainTextEdit::Fields::Text] = "text";
            PlainTextEdit* edit = new PlainTextEdit(descr, GetAccessor(), Reflection::Create(&model));
            edit->SetObjectName("PlainTextEdit");
            layout->AddControl(edit);
        }

        Reflection refModel = Reflection::Create(&dataSource);
        {
            ControlDescriptorBuilder<PlainTextEdit::Fields> descr;
            descr[PlainTextEdit::Fields::Text] = "readOnlyTextMeta";
            PlainTextEdit* edit = new PlainTextEdit(descr, GetAccessor(), refModel);
            edit->SetObjectName("PlainTextEdit_readOnlyMeta");
            layout->AddControl(edit);
        }

        {
            ControlDescriptorBuilder<PlainTextEdit::Fields> descr;
            descr[PlainTextEdit::Fields::Text] = "readOnlyText";
            PlainTextEdit* edit = new PlainTextEdit(descr, GetAccessor(), refModel);
            edit->SetObjectName("PlainTextEdit_readOnlyText");
            layout->AddControl(edit);
        }

        {
            ControlDescriptorBuilder<PlainTextEdit::Fields> descr;
            descr[PlainTextEdit::Fields::Text] = "text";
            descr[PlainTextEdit::Fields::PlaceHolder] = "placeHolder";
            descr[PlainTextEdit::Fields::IsReadOnly] = "isReadOnly";
            descr[PlainTextEdit::Fields::IsEnabled] = "isEnabled";
            PlainTextEdit* edit = new PlainTextEdit(descr, GetAccessor(), refModel);
            edit->SetObjectName("PlainTextEdit_text");
            layout->AddControl(edit);
        }

        {
            ControlDescriptorBuilder<PlainTextEdit::Fields> descr;
            descr[PlainTextEdit::Fields::Text] = "shortText";
            PlainTextEdit* edit = new PlainTextEdit(descr, GetAccessor(), refModel);
            edit->SetObjectName("PlainTextEdit_shortText");
            layout->AddControl(edit);
        }

        PanelKey key("PlainTextEditPanel", CentralPanelInfo());
        GetUI()->AddView(wndKey, key, w);
    }

    DAVA::Map<DAVA::String, DAVA::Any> model;
    PlainTextEditDataSource dataSource;

    static PlainTextEditTestModule* instance;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(PlainTextEditTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<PlainTextEditTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

PlainTextEditTestModule* PlainTextEditTestModule::instance = nullptr;

void EditText(QWidget* w)
{
    QTestEventList eventList;
    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint(1, 0));
    eventList.addKeyClicks(QString("Text"));
    eventList.addKeyClick(Qt::Key_Enter);
    eventList.simulate(w);
}
}

DAVA_TARC_TESTCLASS(PlainTextEditTest)
{
    QPlainTextEdit* GetPlainTextEdit(const QString& controlName)
    {
        QList<QWidget*> widgets = LookupWidget(PlainTextEditTestDetails::wndKey, controlName);
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QPlainTextEdit* plainTextEdit = qobject_cast<QPlainTextEdit*>(w);
        TEST_VERIFY(plainTextEdit != nullptr);

        return plainTextEdit;
    }

    DAVA_TEST (EditTextTest)
    {
        using namespace PlainTextEditTestDetails;
        using namespace testing;

        QPlainTextEdit* plainTextEdit = GetPlainTextEdit("PlainTextEdit");

        { // simulate text editing
            QTestEventList eventList;
            eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint(1, 0));
            for (int i = 0; i < 5; ++i)
            {
                eventList.addKeyClick(Qt::Key_Delete);
            }

            eventList.addKeyClicks(QString("Reflected line "));
            eventList.addKeyClick(Qt::Key_Enter);
            eventList.simulate(plainTextEdit);
        }

        TEST_VERIFY(plainTextEdit->toPlainText() == QString("Reflected line \n text edit text"));

        QFocusEvent focusEvent(QEvent::FocusOut);
        QCoreApplication::sendEvent(plainTextEdit, &focusEvent);

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke([this] {
            PlainTextEditTestModule* inst = PlainTextEditTestModule::instance;
            TEST_VERIFY(inst->model.find("text") != inst->model.end());
            qDebug() << inst->model["text"].Cast<DAVA::String>().c_str();
            TEST_VERIFY(inst->model["text"].Cast<DAVA::String>() == DAVA::String("Reflected line \n text edit text"));
        }));
    }

    void ReadOnlyTest(const QString& widgetName)
    {
        using namespace PlainTextEditTestDetails;
        using namespace testing;

        QWidget* w = GetPlainTextEdit(widgetName);
        TEST_VERIFY(PlainTextEditTestModule::instance->dataSource.text.empty());
        EditText(w);
        TEST_VERIFY(PlainTextEditTestModule::instance->dataSource.text.empty());
    }

    DAVA_TEST (ReadOnlyMetaTest)
    {
        ReadOnlyTest("PlainTextEdit_readOnlyMeta");
    }

    DAVA_TEST (ReadOnlyTextTest)
    {
        ReadOnlyTest("PlainTextEdit_readOnlyText");
    }

    DAVA_TEST (ShortTest)
    {
        using namespace PlainTextEditTestDetails;
        using namespace testing;

        QPlainTextEdit* plainTextEdit = GetPlainTextEdit("PlainTextEdit_shortText");

        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint(1, 0));
        eventList.addKeyClicks(QString("1234567890"));
        eventList.addKeyClick(Qt::Key_Enter);
        eventList.simulate(plainTextEdit);

        QFocusEvent focusEvent(QEvent::FocusOut);
        QCoreApplication::sendEvent(plainTextEdit, &focusEvent);

        TEST_VERIFY(PlainTextEditTestModule::instance->dataSource.text == "12345");
    }

    void PlainTextEditStateTestFinish()
    {
        using namespace PlainTextEditTestDetails;

        QPlainTextEdit* plainTextEdit = GetPlainTextEdit("PlainTextEdit_text");
        TEST_VERIFY(plainTextEdit->placeholderText() == "new placeHolder");

        TEST_VERIFY(plainTextEdit->isEnabled() == false);
        TEST_VERIFY(plainTextEdit->isReadOnly() == true);
    }

    DAVA_TEST (PlainTextEditStateTest)
    {
        using namespace PlainTextEditTestDetails;
        using namespace testing;

        QPlainTextEdit* plainTextEdit = GetPlainTextEdit("PlainTextEdit_text");

        TEST_VERIFY(plainTextEdit->isReadOnly() == false);
        TEST_VERIFY(plainTextEdit->isEnabled() == true);
        TEST_VERIFY(plainTextEdit->placeholderText() == "placeHolder");

        PlainTextEditTestModule::instance->dataSource.isEnabled = false;
        PlainTextEditTestModule::instance->dataSource.isReadOnly = true;
        PlainTextEditTestModule::instance->dataSource.placeHolder = "new placeHolder";

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &PlainTextEditTest::PlainTextEditStateTestFinish));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(PlainTextEditTestDetails::PlainTextEditTestModule);
    END_TESTED_MODULES()

    DAVA::TArc::QtConnections connections;
};
