#pragma once

#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/FilePathEdit.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Testing/Private/TestModuleHolder.h"

#include <QtTools/Utils/QtDelayedExecutor.h>

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QWidget>
#include <QToolButton>
#include <QApplication>
#include <QtTest>

namespace FilePathEditTestDetails
{
using namespace DAVA;
using namespace DAVA::TArc;

WindowKey wndKey = FastName("FilePathEditWnd");

class TestData : public ReflectionBase
{
public:
    FilePath filePath = "~res:/Materials/2d.Color.material";
    bool isReadOnly = true;
    bool isEnabled = false;

    const FilePath& GetValue() const
    {
        return filePath;
    }

    void SetValue(const FilePath& path)
    {
        filePath = path;
    }

    static DAVA::M::ValidationResult ValidatePath(const DAVA::Any& newValue, const DAVA::Any& oldValue)
    {
        DAVA::M::ValidationResult result;
        result.state = DAVA::M::ValidationResult::eState::Valid;
        DAVA::FilePath path = newValue.Cast<DAVA::FilePath>();
        if (path.Exists() == false)
        {
            result.state = DAVA::M::ValidationResult::eState::Invalid;
        }

        return result;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestData)
    {
        ReflectionRegistrator<TestData>::Begin()
        .Field("readOnlyValue", &TestData::GetValue, nullptr)[DAVA::M::File("Material (*.material)", "Open Material")]
        .Field("readOnlyMetaValue", &TestData::GetValue, &TestData::SetValue)[DAVA::M::ReadOnly(), DAVA::M::File("Material (*.material)", "Open Material")]
        .Field("value", &TestData::GetValue, &TestData::SetValue)[DAVA::M::File("Material (*.material)"), DAVA::M::Validator(&TestData::ValidatePath)]
        .Field("isReadOnly", &TestData::isReadOnly)
        .Field("isEnabled", &TestData::isEnabled)
        .End();
    }
};

class TestModule : public DAVA::TArc::ClientModule
{
public:
    TestModule()
        : holder(this)
    {
    }

    void PostInit() override
    {
        QWidget* centralWidget = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(centralWidget);

        Reflection model = Reflection::Create(&data);

        {
            FilePathEdit::Params params;
            params.ui = GetUI();
            params.wndKey = wndKey;
            params.fields[FilePathEdit::Fields::Value] = "readOnlyValue";
            FilePathEdit* edit = new FilePathEdit(params, GetAccessor(), model);
            edit->SetObjectName("FilePathEdit_readOnlyValue");
            layout->AddControl(edit);
        }

        {
            FilePathEdit::Params params;
            params.ui = GetUI();
            params.wndKey = wndKey;
            params.fields[FilePathEdit::Fields::Value] = "readOnlyMetaValue";
            FilePathEdit* edit = new FilePathEdit(params, GetAccessor(), model);
            edit->SetObjectName("FilePathEdit_readOnlyMetaValue");
            layout->AddControl(edit);
        }

        {
            FilePathEdit::Params params;
            params.ui = GetUI();
            params.wndKey = wndKey;
            params.fields[FilePathEdit::Fields::Value] = "value";
            FilePathEdit* edit = new FilePathEdit(params, GetAccessor(), model);
            edit->SetObjectName("FilePathEdit_value");
            layout->AddControl(edit);
        }

        {
            FilePathEdit::Params params;
            params.ui = GetUI();
            params.wndKey = wndKey;
            params.fields[FilePathEdit::Fields::Value] = "value";
            params.fields[FilePathEdit::Fields::IsEnabled] = "isEnabled";
            params.fields[FilePathEdit::Fields::IsReadOnly] = "isReadOnly";
            FilePathEdit* edit = new FilePathEdit(params, GetAccessor(), model);
            edit->SetObjectName("FilePathEdit_final");
            layout->AddControl(edit);
        }

        GetUI()->AddView(wndKey, PanelKey(QStringLiteral("FilePathEditPanel"), CentralPanelInfo()), centralWidget);
    }

    TestData data;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestModule, DAVA::TArc::ClientModule)
    {
        ReflectionRegistrator<TestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

private:
    TestModuleHolder<TestModule> holder;
};

using Holder = TestModuleHolder<TestModule>;
}

DAVA_TARC_TESTCLASS(FilePathEditTest)
{
    QLineEdit* LookUpEdit(const QString& widgetName)
    {
        QWidget* w = LookupSingleWidget<QWidget>(FilePathEditTestDetails::wndKey, widgetName);
        QLineEdit* e = w->findChild<QLineEdit*>("filePathEdit");
        TEST_VERIFY(e != nullptr);

        return e;
    }

    void ModifyTest(QLineEdit * edit, bool readOnlyMode)
    {
        DAVA::FilePath path = FilePathEditTestDetails::Holder::moduleInstance->data.filePath;

        edit->setCursorPosition(edit->text().size());
        QTestEventList e;
        e.addKeyClick(Qt::Key_Backspace);
        e.simulate(edit);

        if (readOnlyMode)
        {
            TEST_VERIFY(edit->text().toStdString() == FilePathEditTestDetails::Holder::moduleInstance->data.filePath.GetAbsolutePathname());
        }
        else
        {
            TEST_VERIFY(edit->text().toStdString() != FilePathEditTestDetails::Holder::moduleInstance->data.filePath.GetAbsolutePathname());
        }

        e.clear();
        e.addKeyClick(Qt::Key_Enter);
        e.simulate(edit); // deletion last symbol make path on nonexistent file, so data should not be changed
        TEST_VERIFY(path == FilePathEditTestDetails::Holder::moduleInstance->data.filePath);
    }

    DAVA_TEST (ReadOnlyTests)
    {
        using namespace ::testing;
        ModifyTest(LookUpEdit("FilePathEdit_readOnlyMetaValue"), true);
        ModifyTest(LookUpEdit("FilePathEdit_readOnlyValue"), true);
        ModifyTest(LookUpEdit("FilePathEdit_final"), true);
        ModifyTest(LookUpEdit("FilePathEdit_value"), false);

        FilePathEditTestDetails::Holder::moduleInstance->data.isReadOnly = false;
        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke([this]()
                         {
                             ModifyTest(LookUpEdit("FilePathEdit_final"), true);
                             FilePathEditTestDetails::Holder::moduleInstance->data.isEnabled = true;
                             EXPECT_CALL(*this, AfterWrappersSync())
                             .WillOnce(Invoke([this]()
                                              {
                                                  ModifyTest(LookUpEdit("FilePathEdit_final"), false);
                                              }));
                         }));
    }

    DAVA_TEST (ModifyFilePathTest)
    {
        QLineEdit* edit = LookUpEdit("FilePathEdit_value");
        QString text = edit->text();
        int index = text.lastIndexOf('.');
        TEST_VERIFY(index != -1);
        edit->setCursorPosition(index);

        FilePathEditTestDetails::TestModule* module = FilePathEditTestDetails::Holder::moduleInstance;

        DAVA::FilePath cachedValue = module->data.filePath;

        QTestEventList e;
        e.addKeyClick(Qt::Key_Backspace);
        e.addKeyClick(Qt::Key_Backspace);
        e.addKeyClick(Qt::Key_Backspace);
        e.addKeyClick(Qt::Key_Enter);
        e.simulate(edit);

        TEST_VERIFY(cachedValue == module->data.filePath);
        TEST_VERIFY(edit->text().toStdString() == module->data.filePath.GetAbsolutePathname());

        DAVA::FilePath newFilePath("~res:/Materials/2d.AlphaFill.material");
        edit->selectAll();
        e.clear();
        e.addKeyClick(Qt::Key_Delete);
        e.addKeyClicks(QString::fromStdString(newFilePath.GetStringValue()));
        e.addKeyClick(Qt::Key_Enter);
        e.simulate(edit);

        TEST_VERIFY(edit->text().toStdString() == newFilePath.GetAbsolutePathname());
        TEST_VERIFY(module->data.filePath.GetAbsolutePathname() == newFilePath.GetAbsolutePathname());

        edit->setCursorPosition(edit->text().size() - QString(".material").size());
        e.clear();
        for (int i = 0; i < QString("AlphaFill").size(); ++i)
        {
            e.addKeyClick(Qt::Key_Backspace);
        }
        e.addKeyClicks("DistanceFont");
        e.addKeyClick(Qt::Key_Enter);
        e.simulate(edit);

        DAVA::FilePath sdfPath("~res:/Materials/2d.DistanceFont.material");
        TEST_VERIFY(edit->text().toStdString() == sdfPath.GetAbsolutePathname());
        TEST_VERIFY(module->data.filePath.GetAbsolutePathname() == sdfPath.GetAbsolutePathname());
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(FilePathEditTestDetails::TestModule);
    END_TESTED_MODULES()
};
