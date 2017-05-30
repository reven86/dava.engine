#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/ComboBoxCheckable.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/GlobalEnum.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QtTest>
#include <QComboBox>
#include <QStyle>
#include <QStyleOption>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QWidget>

#include <QCoreApplication>
#include <QDebug>

namespace ComboBoxChekableTestDetails
{
DAVA::TArc::WindowKey wndKey = DAVA::FastName("ComboBoxChekableTestWnd");

class ComboBoxCheckableTestModule : public DAVA::TArc::ClientModule
{
public:
    enum eTestedFlags
    {
        none = 0,
        first = 1,
        second = 1 << 1,
        third = 1 << 2,

        all = first | second | third
    };

    class TestModel : public ReflectionBase
    {
    public:
        int value = eTestedFlags::none;

        DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestModel, ReflectionBase)
        {
            DAVA::ReflectionRegistrator<TestModel>::Begin()
            .Field("value", &TestModel::value)[DAVA::M::FlagsT<ComboBoxCheckableTestModule::eTestedFlags>()]
            .End();
        }
    };

    TestModel model;

    ComboBoxCheckableTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;

        DAVA::Reflection reflectedModel = DAVA::Reflection::Create(&model);

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        {
            ComboBoxCheckable::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ComboBoxCheckable::Fields::Value] = "value";
            ComboBoxCheckable* comboBox = new ComboBoxCheckable(params, GetAccessor(), reflectedModel);
            comboBox->SetObjectName("ComboBoxCheckable");
            layout->AddControl(comboBox);
        }

        DAVA::TArc::PanelKey panelKey("ComboBoxCheckableTest", DAVA::TArc::CentralPanelInfo());
        GetUI()->AddView(wndKey, panelKey, w);
    }

    static ComboBoxCheckableTestModule* instance;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ComboBoxCheckableTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<ComboBoxCheckableTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

ComboBoxCheckableTestModule* ComboBoxCheckableTestModule::instance = nullptr;
}

ENUM_DECLARE(ComboBoxChekableTestDetails::ComboBoxCheckableTestModule::eTestedFlags)
{
    ENUM_ADD_DESCR(static_cast<int>(ComboBoxChekableTestDetails::ComboBoxCheckableTestModule::eTestedFlags::first), "1st");
    ENUM_ADD_DESCR(static_cast<int>(ComboBoxChekableTestDetails::ComboBoxCheckableTestModule::eTestedFlags::second), "2nd");
    ENUM_ADD_DESCR(static_cast<int>(ComboBoxChekableTestDetails::ComboBoxCheckableTestModule::eTestedFlags::third), "3rd");
    ENUM_ADD_DESCR(static_cast<int>(ComboBoxChekableTestDetails::ComboBoxCheckableTestModule::eTestedFlags::all), "All");
}

DAVA_TARC_TESTCLASS(ComboBoxCheckableTest)
{
    void ChangeItemState(int row)
    {
        using namespace ComboBoxChekableTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, "ComboBoxCheckable");
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();
        QComboBox* comboBox = qobject_cast<QComboBox*>(w);
        TEST_VERIFY(comboBox != nullptr);
        TEST_VERIFY(comboBox->count() == 4);

        QAbstractItemView* v = comboBox->view();
        QAbstractItemModel* abstractModel = v->model();

        QTestEventList changeStateEvents;
        QModelIndex index = v->model()->index(row, 0);
        v->setCurrentIndex(index);
        changeStateEvents.addMouseClick(Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), v->visualRect(index).center());
        changeStateEvents.simulate(v->viewport());

        QHideEvent hideEvent;
        QCoreApplication::sendEvent(v->viewport(), &hideEvent);
    }

    DAVA_TEST (CheckableTest)
    {
        using namespace ComboBoxChekableTestDetails;
        using namespace ::testing;

        ComboBoxCheckableTestModule* inst = ComboBoxCheckableTestModule::instance;
        TEST_VERIFY(inst->model.value == ComboBoxCheckableTestModule::eTestedFlags::none);

        ChangeItemState(0);
        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke([this]() {
            ComboBoxCheckableTestModule* inst = ComboBoxCheckableTestModule::instance;
            TEST_VERIFY(inst->model.value == ComboBoxCheckableTestModule::eTestedFlags::first);

            ChangeItemState(1);
            EXPECT_CALL(*this, AfterWrappersSync())
            .WillOnce(Invoke([this]() {
                ComboBoxCheckableTestModule* inst = ComboBoxCheckableTestModule::instance;
                TEST_VERIFY(inst->model.value == (ComboBoxCheckableTestModule::eTestedFlags::first | ComboBoxCheckableTestModule::eTestedFlags::second));

                ChangeItemState(2);
                EXPECT_CALL(*this, AfterWrappersSync())
                .WillOnce(Invoke([this]() {
                    ComboBoxCheckableTestModule* inst = ComboBoxCheckableTestModule::instance;
                    TEST_VERIFY(inst->model.value == ComboBoxCheckableTestModule::eTestedFlags::all);
                }));
            }));
        }));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ComboBoxChekableTestDetails::ComboBoxCheckableTestModule);
    END_TESTED_MODULES()
};
