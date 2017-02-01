#include "Classes/DevFuncs/TestUIModule.h"
#include "Classes/Application/REGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/LineEdit.h>
#include <TArc/Controls/IntSpinBox.h>
#include <TArc/Controls/QtBoxLayouts.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <Logger/Logger.h>

#include <QAction>
#include <QDialog>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

namespace TestUIModuleDetails
{
using namespace DAVA;
struct Result
{
    ReflectionBase* model = nullptr;
    QLayout* layout = nullptr;
};

using TestSpaceCreator = Result (*)(TArc::ContextAccessor* accessor, QWidget* parent);

struct CheckBoxTestData : public ReflectionBase
{
    bool GetInvertedValue() const
    {
        return !value;
    }
    void SetInvertedValue(bool v)
    {
        Logger::Info("++++ newValue: %d", !v);
        value = !v;
    }

    String GetInvertedDescription()
    {
        return value == true ? "False" : "True";
    }
    bool value = false;

    static String ValueDescription(const Any& v)
    {
        bool value = v.Cast<bool>();
        return value == true ? "True" : "False";
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(CheckBoxTestData, ReflectionBase)
    {
        ReflectionRegistrator<CheckBoxTestData>::Begin()
        .Field("value", &CheckBoxTestData::value)[DAVA::M::ValueDescription(&CheckBoxTestData::ValueDescription)]
        .Field("invertedValue", &CheckBoxTestData::GetInvertedValue, &CheckBoxTestData::SetInvertedValue)
        .Field("valueDescription", &CheckBoxTestData::GetInvertedDescription, nullptr)
        .End();
    }

    static Result Create(TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        Result r;
        CheckBoxTestData* data = new CheckBoxTestData();
        r.model = data;
        r.layout = new QVBoxLayout();
        Reflection reflection = Reflection::Create(data);

        ControlDescriptorBuilder<CheckBox::Fields> descr;
        descr[CheckBox::Fields::Checked] = "value";
        CheckBox* valueBox = new CheckBox(descr, accessor, reflection, parent); //->ToWidgetCast();
        r.layout->addWidget(valueBox->ToWidgetCast());

        ControlDescriptorBuilder<CheckBox::Fields> valueInvertedDescr;
        valueInvertedDescr[CheckBox::Fields::Checked] = "invertedValue";
        valueInvertedDescr[CheckBox::Fields::TextHint] = "valueDescription";
        valueInvertedDescr[CheckBox::Fields::IsReadOnly] = "value";
        CheckBox* valueInvertedBox = new CheckBox(valueInvertedDescr, accessor, reflection, parent); //->ToWidgetCast();
        r.layout->addWidget(valueInvertedBox->ToWidgetCast());

        return r;
    }
};

struct LineEditTestData : public ReflectionBase
{
    String text = "Text in line edit";
    String placeHolder = "Empty string";
    bool isReadOnly = false;
    bool isEnabled = true;

    String GetText() const
    {
        return text;
    }

    void SetText(const String& t)
    {
        text = t;
    }

    static String GetReadOnlyDescription(const Any& v)
    {
        bool value = v.Cast<bool>();
        return value == true ? "Read only" : "Writable";
    }

    static String GetEnableDescription(const Any& v)
    {
        bool value = v.Cast<bool>();
        return value == true ? "Enabled" : "Disabled";
    }

    static M::ValidationResult ValidateText(const Any& value, const Any& prevValue)
    {
        String v = value.Cast<String>();
        String pv = prevValue.Cast<String>();
        M::ValidationResult r;
        r.state = M::ValidationResult::eState::Valid;

        if (v.size() > 20)
        {
            r.state = M::ValidationResult::eState::Invalid;
            r.message = "Too long text!";
        }

        return r;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LineEditTestData, ReflectionBase)
    {
        using namespace DAVA::M;

        ReflectionRegistrator<LineEditTestData>::Begin()
        .Field("readOnlyMetaText", &LineEditTestData::text)[ReadOnly()]
        .Field("readOnlyText", &LineEditTestData::GetText, nullptr)
        .Field("text", &LineEditTestData::GetText, &LineEditTestData::SetText)[Validator(&LineEditTestData::ValidateText)]
        .Field("isTextReadOnly", &LineEditTestData::isReadOnly)[DAVA::M::ValueDescription(&LineEditTestData::GetReadOnlyDescription)]
        .Field("isTextEnabled", &LineEditTestData::isEnabled)[DAVA::M::ValueDescription(&LineEditTestData::GetEnableDescription)]
        .Field("placeholder", &LineEditTestData::placeHolder)
        .End();
    }

    static Result Create(TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;
        LineEditTestData* data = new LineEditTestData();
        QVBoxLayout* boxLayout = new QVBoxLayout();

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Meta read only : ", parent));

            ControlDescriptorBuilder<LineEdit::Fields> desr;
            desr[LineEdit::Fields::Text] = "readOnlyMetaText";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Read only : ", parent));

            ControlDescriptorBuilder<LineEdit::Fields> desr;
            desr[LineEdit::Fields::Text] = "readOnlyText";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Editable : ", parent));

            ControlDescriptorBuilder<LineEdit::Fields> desr;
            desr[LineEdit::Fields::Text] = "text";
            desr[LineEdit::Fields::PlaceHolder] = "placeholder";
            desr[LineEdit::Fields::IsReadOnly] = "isTextReadOnly";
            desr[LineEdit::Fields::IsEnabled] = "isTextEnabled";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Fields::Checked] = "isTextReadOnly";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Is enabled
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Fields::Checked] = "isTextEnabled";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);

                QVBoxLayout* vbox = new QVBoxLayout(checkBox->ToWidgetCast());
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }
        }

        Result r;
        r.model = data;
        r.layout = boxLayout;
        return r;
    }
};

struct IntSpinBoxTestData : public ReflectionBase
{
    int v = 10;

    Any GetValue() const
    {
        if (v == 10)
        {
            return Any();
        }
        return Any(v);
    }

    void SetValue(const Any& value)
    {
        v = value.Cast<int>();
    }

    String GetTextValue() const
    {
        return std::to_string(v);
    }

    DAVA_VIRTUAL_REFLECTION(IntSpinBoxTestData)
    {
        ReflectionRegistrator<IntSpinBoxTestData>::Begin()
        .Field("value", &IntSpinBoxTestData::GetValue, &IntSpinBoxTestData::SetValue)[M::Range(0, 300, 2)]
        .Field("text", &IntSpinBoxTestData::GetTextValue, nullptr)
        .End();
    }

    static Result Create(TArc::ContextAccessor* accessor, QWidget* parent)
    {
        using namespace DAVA::TArc;

        Result r;
        IntSpinBoxTestData* data = new IntSpinBoxTestData();
        r.model = data;
        QtVBoxLayout* layout = new QtVBoxLayout();
        r.layout = layout;
        Reflection reflection = Reflection::Create(data);

        {
            ControlDescriptorBuilder<IntSpinBox::Fields> descr;
            descr[IntSpinBox::Fields::Value] = "value";
            IntSpinBox* spinBox = new IntSpinBox(descr, accessor, reflection, parent);
            layout->AddWidget(spinBox);
        }

        {
            ControlDescriptorBuilder<LineEdit::Fields> d;
            d[LineEdit::Fields::Text] = "text";
            layout->AddWidget(new LineEdit(d, accessor, reflection, parent));
        }

        return r;
    }
};

struct Node
{
    TestSpaceCreator creator;
    QString title;
};
}

void TestUIModule::PostInit()
{
    using namespace DAVA::TArc;
    UI* ui = GetUI();
    ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "DebugFunctions"));

    QAction* assertAction = new QAction("UI Sandbox", nullptr);
    connections.AddConnection(assertAction, &QAction::triggered, DAVA::MakeFunction(this, &TestUIModule::ShowDialog));
    ui->AddAction(REGlobal::MainWindowKey, placementInfo, assertAction);
}

void TestUIModule::ShowDialog()
{
    using namespace TestUIModuleDetails;
    QDialog* dlg = new QDialog();
    QGridLayout* layout = new QGridLayout(dlg);
    dlg->setLayout(layout);

    DAVA::Vector<Node> nodes =
    {
      { &CheckBoxTestData::Create, "CheckBox Test" },
      { &LineEditTestData::Create, "LineEdit Test" },
      { &IntSpinBoxTestData::Create, "SpinBoxTest" }
    };
    DAVA::Vector<DAVA::ReflectionBase*> data;

    const int columnCount = 6;
    int currentColumn = 0;
    int currentRow = 0;
    DAVA::TArc::ContextAccessor* accessor = GetAccessor();
    for (Node& node : nodes)
    {
        QGroupBox* groupBox = new QGroupBox(node.title, dlg);

        Result r = node.creator(accessor, groupBox);
        data.push_back(r.model);

        groupBox->setLayout(r.layout);
        layout->addWidget(groupBox, currentRow, currentColumn);
        ++currentColumn;
        if (currentColumn > columnCount)
        {
            currentColumn = 0;
            ++currentRow;
        }
    }

    dlg->exec();
    delete dlg;
    for (ReflectionBase* d : data)
    {
        delete d;
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(TestUIModule)
{
    DAVA::ReflectionRegistrator<TestUIModule>::Begin()
    .ConstructorByPointer()
    .End();
}

#if !defined(DEPLOY_BUILD)
DECL_GUI_MODULE(TestUIModule);
#endif
