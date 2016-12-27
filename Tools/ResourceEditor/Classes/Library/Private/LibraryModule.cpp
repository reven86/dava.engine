#include "Classes/Library/LibraryModule.h"
#include "Classes/Library/Private/ControlsFactory.h"
#include "Classes/Library/Private/LibraryData.h"
#include "Classes/Library/Private/LibraryWidget.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/Actions/DAEConverter.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include "TArc/Core/FieldBinder.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/QtAction.h"

#include "FileSystem/FilePath.h"
#include "Functional/Function.h"

LibraryModule::~LibraryModule()
{
    ControlsFactory::ReleaseFonts();
}

void LibraryModule::PostInit()
{
    std::unique_ptr<LibraryData> libraryData = std::make_unique<LibraryData>();
    GetAccessor()->GetGlobalContext()->CreateData(std::move(libraryData));

    LibraryWidget* libraryWidget = new LibraryWidget(GetAccessor(), nullptr);
    connections.AddConnection(libraryWidget, &LibraryWidget::AddSceneRequested, DAVA::MakeFunction(this, &LibraryModule::OnAddSceneRequested));
    connections.AddConnection(libraryWidget, &LibraryWidget::EditSceneRequested, DAVA::MakeFunction(this, &LibraryModule::OnEditSceneRequested));
    connections.AddConnection(libraryWidget, &LibraryWidget::DAEConvertionRequested, DAVA::MakeFunction(this, &LibraryModule::OnDAEConvertionRequested));
    connections.AddConnection(libraryWidget, &LibraryWidget::DoubleClicked, DAVA::MakeFunction(this, &LibraryModule::OnDoubleClicked));
    connections.AddConnection(libraryWidget, &LibraryWidget::DragStarted, DAVA::MakeFunction(this, &LibraryModule::OnDragStarted));

    DAVA::TArc::DockPanelInfo dockInfo;
    dockInfo.title = "Library";
    DAVA::TArc::PanelKey panelKey(QStringLiteral("LibraryDock"), dockInfo);
    GetUI()->AddView(REGlobal::MainWindowKey, panelKey, libraryWidget);

    fieldBinder.reset(new DAVA::TArc::FieldBinder(GetAccessor()));
    DAVA::TArc::FieldDescriptor libraryFieldDescriptor(DAVA::ReflectedTypeDB::Get<LibraryData>(), DAVA::FastName(LibraryData::selectedPathProperty));
    fieldBinder->BindField(libraryFieldDescriptor, DAVA::MakeFunction(this, &LibraryModule::OnSelectedPathChanged));
}

void LibraryModule::OnSelectedPathChanged(const DAVA::Any& selectedPathValue)
{
    if (SettingsManager::GetValue(Settings::General_PreviewEnabled).AsBool() == true)
    {
        DAVA::FilePath selectedPath;
        if (selectedPathValue.CanGet<DAVA::FilePath>())
        {
            selectedPath = selectedPathValue.Get<DAVA::FilePath>();
        }

        if (selectedPath.IsEqualToExtension(".sc2"))
        {
            ShowPreview(selectedPath);
        }
        else
        {
            HidePreview();
        }
    }
}

void LibraryModule::ShowPreview(const DAVA::FilePath& path)
{
    if (previewDialog.Get() == nullptr)
    {
        previewDialog.Set(new ScenePreviewDialog());
    }

    previewDialog->Show(path);
}

void LibraryModule::HidePreview()
{
    if (previewDialog.Get() != nullptr && previewDialog->GetParent())
    {
        previewDialog->Close();
    }
}

void LibraryModule::OnAddSceneRequested(const DAVA::FilePath& scenePathname)
{
    HidePreview();

    SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
    if (sceneData)
    {
        DAVA::RefPtr<SceneEditor2> sceneEditor = sceneData->GetScene();

        DAVA::TArc::UI* ui = GetUI();
        DAVA::TArc::WaitDialogParams waitDlgParams;
        waitDlgParams.message = QString("Add object to scene\n%1").arg(scenePathname.GetAbsolutePathname().c_str());
        waitDlgParams.needProgressBar = false;
        std::unique_ptr<DAVA::TArc::WaitHandle> waitHandle = ui->ShowWaitDialog(REGlobal::MainWindowKey, waitDlgParams);

        sceneEditor->structureSystem->Add(scenePathname);
    }
}

void LibraryModule::OnEditSceneRequested(const DAVA::FilePath& scenePathname)
{
    HidePreview();

    InvokeOperation(REGlobal::OpenSceneOperation.ID, scenePathname);
}

void LibraryModule::OnDAEConvertionRequested(const DAVA::FilePath& daePathname)
{
    HidePreview();

    DAVA::TArc::UI* ui = GetUI();
    DAVA::TArc::WaitDialogParams waitDlgParams;
    waitDlgParams.message = QString("DAE to SC2 conversion\n%1").arg(daePathname.GetAbsolutePathname().c_str());
    waitDlgParams.needProgressBar = false;
    std::unique_ptr<DAVA::TArc::WaitHandle> waitHandle = ui->ShowWaitDialog(REGlobal::MainWindowKey, waitDlgParams);

    DAEConverter::Convert(daePathname);
}

void LibraryModule::OnDoubleClicked(const DAVA::FilePath& scenePathname)
{
    HidePreview();

    if (SettingsManager::GetValue(Settings::General_OpenByDBClick).AsBool() && scenePathname.IsEqualToExtension(".sc2"))
    {
        OnEditSceneRequested(scenePathname);
    }
}

void LibraryModule::OnDragStarted()
{
    HidePreview();
}
