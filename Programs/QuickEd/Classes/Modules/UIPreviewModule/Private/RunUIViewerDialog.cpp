#include "Modules/UIPreviewModule/Private/RunUIViewerDialog.h"
#include "Modules/ProjectModule/ProjectData.h"

#include <TArc/Controls/LineEdit.h>
#include <TArc/Controls/FilePathEdit.h>
#include <TArc/Controls/QtBoxLayouts.h>

#include <TArc/Controls/Label.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Qt/QtString.h>

#include <Platform/Process.h>

#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/Reflection.h>

#include <QPushButton>

struct RunUIViewerDialog::RunData : public DAVA::ReflectionBase
{
    DAVA::FilePath uiViewerPath;

    DAVA::FilePath projectPath;
    DAVA::FilePath holderYaml;
    DAVA::String holderRoot;
    DAVA::String holderName;

    DAVA::FilePath testedYaml;
    DAVA::String testedName;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(RunData, DAVA::ReflectionBase)
    {
        DAVA::ReflectionRegistrator<RunData>::Begin()
        
#if defined(__DAVAENGINE_MACOS__)
        .Field("uiViewerPath", &RunData::uiViewerPath)[DAVA::M::File("Application (*.app)", "Select UIViewer.app")]
#elif defined(__DAVAENGINE_WIN32__) //
        .Field("uiViewerPath", &RunData::uiViewerPath)[DAVA::M::File("Application (*.exe)", "Select UIViewer.exe")]
#endif //
        .Field("projectPath", &RunData::projectPath)[DAVA::M::ReadOnly()]
        .Field("holderYaml", &RunData::holderYaml)[DAVA::M::File("Yaml (*.yaml)", "Open Holder Yaml")]
        .Field("holderRoot", &RunData::holderRoot)
        .Field("holderName", &RunData::holderName)
        .Field("testedYaml", &RunData::testedYaml)[DAVA::M::File("Yaml (*.yaml)", "Open Tested Yaml")]
        .Field("testedName", &RunData::testedName)
        .End();
    }
};

RunUIViewerDialog::RunUIViewerDialog(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui, QWidget* parent)
    : QDialog(parent)
    , runData(new RunData())
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    {
        ProjectData* projectData = accessor->GetGlobalContext()->GetData<ProjectData>();
        DVASSERT(nullptr != projectData);
        runData->projectPath = projectData->GetProjectDirectory();
    }

    Reflection reflectedModel = Reflection::Create(runData.get());
    QVBoxLayout* boxLayout = new QVBoxLayout();

    {
        QtHBoxLayout* lineLayout = new QtHBoxLayout();
        lineLayout->addWidget(new QLabel("UIViewer Path: ", this));

        FilePathEdit::Params p(accessor, ui, DAVA::TArc::mainWindowKey);
        p.fields[FilePathEdit::Fields::Value] = "uiViewerPath";
        lineLayout->AddControl(new FilePathEdit(p, accessor, reflectedModel, this));
        boxLayout->addLayout(lineLayout);
    }

    {
        QtHBoxLayout* lineLayout = new QtHBoxLayout();
        lineLayout->addWidget(new QLabel("Project Path: ", this));

        FilePathEdit::Params p(accessor, ui, DAVA::TArc::mainWindowKey);
        p.fields[FilePathEdit::Fields::Value] = "projectPath";
        lineLayout->AddControl(new FilePathEdit(p, accessor, reflectedModel, this));
        boxLayout->addLayout(lineLayout);
    }

    {
        QtHBoxLayout* lineLayout = new QtHBoxLayout();
        lineLayout->addWidget(new QLabel("Holder Yaml: ", this));

        FilePathEdit::Params p(accessor, ui, DAVA::TArc::mainWindowKey);
        p.fields[FilePathEdit::Fields::Value] = "holderYaml";
        lineLayout->AddControl(new FilePathEdit(p, accessor, reflectedModel, this));
        boxLayout->addLayout(lineLayout);
    }

    {
        QHBoxLayout* lineLayout = new QHBoxLayout();
        lineLayout->addWidget(new QLabel("Holder Root: ", this));

        LineEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[LineEdit::Fields::Text] = "holderRoot";
        LineEdit* lineEdit = new LineEdit(params, accessor, reflectedModel, this);
        lineLayout->addWidget(lineEdit->ToWidgetCast());
        boxLayout->addLayout(lineLayout);
    }

    {
        QHBoxLayout* lineLayout = new QHBoxLayout();
        lineLayout->addWidget(new QLabel("Holder Name: ", this));

        LineEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[LineEdit::Fields::Text] = "holderName";
        LineEdit* lineEdit = new LineEdit(params, accessor, reflectedModel, this);
        lineLayout->addWidget(lineEdit->ToWidgetCast());
        boxLayout->addLayout(lineLayout);
    }

    {
        QtHBoxLayout* lineLayout = new QtHBoxLayout();
        lineLayout->addWidget(new QLabel("Tested Yaml: ", this));

        FilePathEdit::Params p(accessor, ui, DAVA::TArc::mainWindowKey);
        p.fields[FilePathEdit::Fields::Value] = "testedYaml";
        lineLayout->AddControl(new FilePathEdit(p, accessor, reflectedModel, this));
        boxLayout->addLayout(lineLayout);
    }

    {
        QHBoxLayout* lineLayout = new QHBoxLayout();
        lineLayout->addWidget(new QLabel("Tested Name: ", this));

        LineEdit::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[LineEdit::Fields::Text] = "testedName";
        LineEdit* lineEdit = new LineEdit(params, accessor, reflectedModel, this);
        lineLayout->addWidget(lineEdit->ToWidgetCast());
        boxLayout->addLayout(lineLayout);
    }

    {
        QPushButton* btnOk = new QPushButton(QStringLiteral("Run UIPreview"), this);
        btnOk->setObjectName(QStringLiteral("btnOk"));
        boxLayout->addWidget(btnOk);

        connect(btnOk, &QPushButton::clicked, this, &RunUIViewerDialog::OnOk);
    }

    setLayout(boxLayout);
}

RunUIViewerDialog::~RunUIViewerDialog() = default;

void RunUIViewerDialog::OnOk()
{
    using namespace DAVA;

    if ((runData->uiViewerPath.IsEmpty() || runData->projectPath.IsEmpty() || runData->holderYaml.IsEmpty() || runData->testedYaml.IsEmpty())
        || (runData->holderRoot.empty() || runData->holderName.empty() || runData->testedName.empty()))
    {
        Logger::Error("Please, Fill all Data");
        return;
    }

    FilePath appPath = runData->uiViewerPath;
#if defined(__DAVAENGINE_MACOS__)
    appPath += "/Contents/MacOS/" + runData->uiViewerPath.GetBasename();
#endif //

    Vector<String> args;
    args.emplace_back("options");

    args.emplace_back("-project");
    args.emplace_back(runData->projectPath.GetStringValue());

    args.emplace_back("-holderYaml");
    args.emplace_back(runData->holderYaml.GetFrameworkPath());

    args.emplace_back("-holderRoot");
    args.emplace_back(runData->holderRoot);

    args.emplace_back("-holderCtrl");
    args.emplace_back(runData->holderName);

    args.emplace_back("-testedYaml");
    args.emplace_back(runData->testedYaml.GetFrameworkPath());

    args.emplace_back("-testedCtrl");
    args.emplace_back(runData->testedName);

    Process process(appPath, args);
    if (process.Run(false))
    {
        process.Wait();

        const String& procOutput = process.GetOutput();
        if (procOutput.size() > 0)
        {
            Logger::FrameworkDebug(procOutput.c_str());
        }

        if (process.GetExitCode() != 0)
        {
            Logger::Error("Process exited with code %d", process.GetExitCode());
        }
    }
    else
    {
        Logger::Error("Failed to run %s", appPath.GetStringValue().c_str());
    }

    //    accept();
}
