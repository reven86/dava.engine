#include "Classes/DevFuncs/TestUIModule.h"
#include "Classes/Application/REGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/LineEdit.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/MetaObjects.h>

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
    bool value = false;

    DAVA_VIRTUAL_REFLECTION(CheckBoxTestData, ReflectionBase)
    {
        ReflectionRegistrator<CheckBoxTestData>::Begin()
        .Field("value", &CheckBoxTestData::value)
        .Field("invertedValue", &CheckBoxTestData::GetInvertedValue, &CheckBoxTestData::SetInvertedValue)
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
        descr[CheckBox::Checked] = "value";
        CheckBox* valueBox = new CheckBox(descr, accessor, reflection, parent); //->ToWidgetCast();
        r.layout->addWidget(valueBox->ToWidgetCast());

        ControlDescriptorBuilder<CheckBox::Fields> valueInvertedDescr;
        valueInvertedDescr[CheckBox::Checked] = "invertedValue";
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

    static M::ValidatorResult ValidateText(const Any& value, const Any& prevValue)
    {
        String v = value.Cast<String>();
        String pv = prevValue.Cast<String>();
        M::ValidatorResult r;
        r.state = M::ValidatorResult::eState::Valid;

        if (v.size() > 20)
        {
            r.state = M::ValidatorResult::eState::Valid;
            r.message = "Too long text!";
            std::replace(v.begin(), v.end(), '1', '2');
            r.fixedValue = v;
        }

        return r;
    }

    DAVA_VIRTUAL_REFLECTION(LineEditTestData, ReflectionBase)
    {
        using namespace DAVA::M;

        ReflectionRegistrator<LineEditTestData>::Begin()
        .Field("readOnlyMetaText", &LineEditTestData::text)[ReadOnly()]
        .Field("readOnlyText", &LineEditTestData::GetText, nullptr)
        .Field("text", &LineEditTestData::GetText, &LineEditTestData::SetText)[Validator(&LineEditTestData::ValidateText)]
        .Field("isTextReadOnly", &LineEditTestData::isReadOnly)
        .Field("isTextEnabled", &LineEditTestData::isEnabled)
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
            desr[LineEdit::Text] = "readOnlyMetaText";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Read only : ", parent));

            ControlDescriptorBuilder<LineEdit::Fields> desr;
            desr[LineEdit::Text] = "readOnlyText";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);
        }

        {
            QHBoxLayout* lineLayout = new QHBoxLayout();
            lineLayout->addWidget(new QLabel("Editable : ", parent));

            ControlDescriptorBuilder<LineEdit::Fields> desr;
            desr[LineEdit::Text] = "text";
            desr[LineEdit::PlaceHolder] = "placeholder";
            desr[LineEdit::IsReadOnly] = "isTextReadOnly";
            desr[LineEdit::IsEnabled] = "isTextEnabled";
            LineEdit* lineEdit = new LineEdit(desr, accessor, Reflection::Create(data), parent);
            lineLayout->addWidget(lineEdit->ToWidgetCast());
            boxLayout->addLayout(lineLayout);

            {
                // Read only check box
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Checked] = "isTextReadOnly";
                CheckBox* checkBox = new CheckBox(descr, accessor, Reflection::Create(data), parent);
                boxLayout->addWidget(checkBox->ToWidgetCast());
            }

            {
                // Is enabled
                ControlDescriptorBuilder<CheckBox::Fields> descr;
                descr[CheckBox::Checked] = "isTextEnabled";
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
      { &LineEditTestData::Create, "LineEdit Test" }
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

DAVA_REFLECTION_IMPL(TestUIModule)
{
    DAVA::ReflectionRegistrator<TestUIModule>::Begin()
    .ConstructorByPointer()
    .End();
}

#if !defined(DEPLOY_BUILD)
DECL_GUI_MODULE(TestUIModule);
#endif
