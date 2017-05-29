#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QtTest>

namespace LineEditTestDetails
{
using namespace DAVA;

DAVA::TArc::WindowKey wndKey = DAVA::FastName("LineEditTestWnd");

struct LineEditDataSource
{
    String text = "";
    String placeHolder = "placeHolder";
    bool isReadOnly = false;
    bool isEnabled = true;

    const String& GetText() const
    {
        return text;
    }

    static M::ValidationResult Invalidate(const Any& newValue, const Any& currentValue)
    {
        M::ValidationResult result;
        result.state = M::ValidationResult::eState::Valid;
        String v = newValue.Cast<String>();
        if (v.find('+') != String::npos)
        {
            result.state = M::ValidationResult::eState::Invalid;
        }

        return result;
    }

    static M::ValidationResult FixupValidator(const Any& newValue, const Any& current)
    {
        M::ValidationResult result;
        result.state = M::ValidationResult::eState::Valid;
        String v = newValue.Cast<String>();
        if (v.size() > 5)
        {
            result.state = M::ValidationResult::eState::Invalid;
        }

        String v1 = v;
        std::replace(v.begin(), v.end(), '4', '2');
        if (v1 != v)
        {
            result.state = M::ValidationResult::eState::Intermediate;
            result.fixedValue = v;
        }

        return result;
    }

    DAVA_REFLECTION(LineEditDataSource)
    {
        ReflectionRegistrator<LineEditDataSource>::Begin()
        .Field("readOnlyTextMeta", &LineEditDataSource::text)[M::ReadOnly()]
        .Field("readOnlyText", &LineEditDataSource::GetText, nullptr)
        .Field("text", &LineEditDataSource::text)
        .Field("placeHolder", &LineEditDataSource::placeHolder)
        .Field("isReadOnly", &LineEditDataSource::isReadOnly)
        .Field("isEnabled", &LineEditDataSource::isEnabled)
        .Field("invalidateText", &LineEditDataSource::text)[M::Validator(&LineEditDataSource::Invalidate)]
        .Field("fixUpText", &LineEditDataSource::text)[M::Validator(&LineEditDataSource::FixupValidator)]
        .End();
    }
};

class LineEditTestModule : public DAVA::TArc::ClientModule
{
public:
    LineEditTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;
        model.emplace("text", DAVA::String("Line edit text"));

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        {
            LineEdit::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[LineEdit::Fields::Text] = "text";
            LineEdit* edit = new LineEdit(params, GetAccessor(), Reflection::Create(&model));
            edit->SetObjectName("LineEdit");
            layout->AddControl(edit);
        }

        Reflection refModel = Reflection::Create(&dataSource);
        {
            LineEdit::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[LineEdit::Fields::Text] = "readOnlyTextMeta";
            LineEdit* edit = new LineEdit(params, GetAccessor(), refModel);
            edit->SetObjectName("LineEdit_readOnlyMeta");
            layout->AddControl(edit);
        }

        {
            LineEdit::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[LineEdit::Fields::Text] = "readOnlyText";
            LineEdit* edit = new LineEdit(params, GetAccessor(), refModel);
            edit->SetObjectName("LineEdit_readOnlyText");
            layout->AddControl(edit);
        }

        {
            LineEdit::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[LineEdit::Fields::Text] = "text";
            params.fields[LineEdit::Fields::PlaceHolder] = "placeHolder";
            params.fields[LineEdit::Fields::IsReadOnly] = "isReadOnly";
            params.fields[LineEdit::Fields::IsEnabled] = "isEnabled";
            LineEdit* edit = new LineEdit(params, GetAccessor(), refModel);
            edit->SetObjectName("LineEdit_text");
            layout->AddControl(edit);
        }

        {
            LineEdit::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[LineEdit::Fields::Text] = "invalidateText";
            LineEdit* edit = new LineEdit(params, GetAccessor(), refModel);
            edit->SetObjectName("LineEdit_invalidate");
            layout->AddControl(edit);
        }

        {
            LineEdit::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[LineEdit::Fields::Text] = "fixUpText";
            LineEdit* edit = new LineEdit(params, GetAccessor(), refModel);
            edit->SetObjectName("LineEdit_fixup");
            layout->AddControl(edit);
        }

        PanelKey key("LineEditPanel", CentralPanelInfo());
        GetUI()->AddView(wndKey, key, w);
    }

    DAVA::Map<DAVA::String, DAVA::Any> model;
    LineEditDataSource dataSource;

    static LineEditTestModule* instance;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LineEditTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<LineEditTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

LineEditTestModule* LineEditTestModule::instance = nullptr;

void EditText(QWidget* w)
{
    QTestEventList eventList;
    eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint(1, 0));
    eventList.addKeyClicks(QString("Text"));
    eventList.addKeyClick(Qt::Key_Enter);
    eventList.simulate(w);
}
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

    void ReadOnlyTest(const QString& widgetName)
    {
        using namespace LineEditTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, widgetName);
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        TEST_VERIFY(LineEditTestModule::instance->dataSource.text.empty());
        EditText(w);
        TEST_VERIFY(LineEditTestModule::instance->dataSource.text.empty());
    }

    DAVA_TEST (ReadOnlyMetaTest)
    {
        ReadOnlyTest("LineEdit_readOnlyMeta");
    }

    DAVA_TEST (ReadOnlyTextTest)
    {
        ReadOnlyTest("LineEdit_readOnlyText");
    }

    DAVA_TEST (InvalidateTest)
    {
        using namespace LineEditTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("LineEdit_invalidate"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint(1, 0));
        eventList.addKeyClicks(QString("4+2+3"));
        eventList.addKeyClick(Qt::Key_Enter);
        eventList.simulate(w);

        TEST_VERIFY(LineEditTestModule::instance->dataSource.text == "423");
    }

    DAVA_TEST (FixUpTest)
    {
        using namespace LineEditTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("LineEdit_fixup"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(w);

        QTestEventList eventList;
        eventList.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint(1, 0));
        eventList.addKeyClicks(QString("555"));
        eventList.addKeyClick(Qt::Key_Enter);
        eventList.simulate(w);

        TEST_VERIFY(LineEditTestModule::instance->dataSource.text == "55223");
    }

    void LineEditStateTestFinish()
    {
        using namespace LineEditTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("LineEdit_text"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(w);

        TEST_VERIFY(lineEdit->placeholderText() == "new placeHolder");
        TEST_VERIFY(lineEdit->isEnabled() == false);
        TEST_VERIFY(lineEdit->isReadOnly() == true);
    }

    DAVA_TEST (LineEditStateTest)
    {
        using namespace LineEditTestDetails;
        using namespace testing;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString("LineEdit_text"));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(w);

        TEST_VERIFY(lineEdit->isReadOnly() == false);
        TEST_VERIFY(lineEdit->isEnabled() == true);
        TEST_VERIFY(lineEdit->placeholderText() == "placeHolder");

        LineEditTestModule::instance->dataSource.isEnabled = false;
        LineEditTestModule::instance->dataSource.isReadOnly = true;
        LineEditTestModule::instance->dataSource.placeHolder = "new placeHolder";

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &LineEditTest::LineEditStateTestFinish));
    }

    MOCK_METHOD1_VIRTUAL(OnTextChanged, void(const QString& s));
    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(LineEditTestDetails::LineEditTestModule);
    END_TESTED_MODULES()

    DAVA::TArc::QtConnections connections;
};
