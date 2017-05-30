#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/Label.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/CommonStrings.h"

#include "TArc/Qt/QtString.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Math/Matrix2.h>
#include <Math/Matrix3.h>
#include <Math/Matrix4.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QtTest>

namespace LabelTestDetails
{
using namespace DAVA;

DAVA::TArc::WindowKey wndKey = DAVA::FastName("LabelTestWnd");

String M2ToString(const Matrix2& matrix)
{
    return Format("[%f, %f]\n[%f, %f]",
                  matrix._data[0][0], matrix._data[0][1],
                  matrix._data[1][0], matrix._data[1][1]);
}

String M3ToString(const Matrix3& matrix)
{
    return Format("[%f, %f, %f]\n[%f, %f, %f]\n[%f, %f, %f]",
                  matrix._data[0][0], matrix._data[0][1], matrix._data[0][2],
                  matrix._data[1][0], matrix._data[1][1], matrix._data[1][2],
                  matrix._data[2][0], matrix._data[2][1], matrix._data[2][2]
                  );
}

String M4ToString(const Matrix4& matrix)
{
    return Format("[%f, %f, %f, %f]\n[%f, %f, %f, %f]\n[%f, %f, %f, %f]\n[%f, %f, %f, %f]",
                  matrix._data[0][0], matrix._data[0][1], matrix._data[0][2], matrix._data[0][3],
                  matrix._data[1][0], matrix._data[1][1], matrix._data[1][2], matrix._data[1][3],
                  matrix._data[2][0], matrix._data[2][1], matrix._data[2][2], matrix._data[2][3],
                  matrix._data[3][0], matrix._data[3][1], matrix._data[3][2], matrix._data[3][3]
                  );
}

struct LabelDataSource
{
    String text = "test String";
    QString qtext = "test QString";

    const String& GetText() const
    {
        return text;
    }

    Any multipleValues;

    Matrix2 m2;
    Matrix3 m3;
    Matrix4 m4;

    DAVA_REFLECTION(LabelDataSource)
    {
        ReflectionRegistrator<LabelDataSource>::Begin()
        .Field("text", &LabelDataSource::text)
        .Field("qtext", &LabelDataSource::qtext)
        .Field("getText", &LabelDataSource::GetText, nullptr)
        .Field("multi", &LabelDataSource::multipleValues)
        .Field("m2", &LabelDataSource::m2)
        .Field("m3", &LabelDataSource::m3)
        .Field("m4", &LabelDataSource::m4)
        .End();
    }
};

class LabelTestModule : public DAVA::TArc::ClientModule
{
public:
    LabelTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        using namespace DAVA::TArc;

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        auto createLabel = [this, layout](const String& fieldName)
        {
            Reflection refModel = Reflection::Create(&dataSource);

            Label::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[Label::Fields::Text] = fieldName.c_str();
            Label* label = new Label(params, GetAccessor(), refModel);
            label->SetObjectName(QString::fromStdString("Label_" + fieldName));
            layout->AddControl(label);
        };

        createLabel("text");
        createLabel("qtext");
        createLabel("getText");
        createLabel("multi");
        createLabel("m2");
        createLabel("m3");
        createLabel("m4");

        PanelKey key("LabelPanel", CentralPanelInfo());
        GetUI()->AddView(wndKey, key, w);
    }

    LabelDataSource dataSource;

    static LabelTestModule* instance;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LabelTestModule, DAVA::TArc::ClientModule)
    {
        DAVA::ReflectionRegistrator<LabelTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

LabelTestModule* LabelTestModule::instance = nullptr;
}

DAVA_TARC_TESTCLASS(LabelTest)
{
    QLabel* GetLabel(const DAVA::String& fieldName)
    {
        using namespace LabelTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, QString::fromStdString("Label_" + fieldName));
        TEST_VERIFY(widgets.size() == 1);
        QWidget* w = widgets.front();

        QLabel* label = qobject_cast<QLabel*>(w);
        TEST_VERIFY(label != nullptr);

        return label;
    }

    DAVA_TEST (LabelTextTest)
    {
        using namespace LabelTestDetails;

        QLabel* label = GetLabel("text");
        TEST_VERIFY(label->text() == "test String");

        label = GetLabel("qtext");
        TEST_VERIFY(label->text() == "test QString");

        label = GetLabel("getText");
        TEST_VERIFY(label->text() == "test String");

        label = GetLabel("multi");
        TEST_VERIFY(label->text() == QString(DAVA::TArc::MultipleValuesString));

        LabelTestModule* inst = LabelTestModule::instance;

        label = GetLabel("m2");
        TEST_VERIFY(label->text().toStdString() == M2ToString(inst->dataSource.m2));

        label = GetLabel("m3");
        TEST_VERIFY(label->text().toStdString() == M3ToString(inst->dataSource.m3));

        label = GetLabel("m4");
        TEST_VERIFY(label->text().toStdString() == M4ToString(inst->dataSource.m4));
    }

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(LabelTestDetails::LabelTestModule);
    END_TESTED_MODULES()
};
