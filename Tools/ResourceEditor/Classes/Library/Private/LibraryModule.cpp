#include "Classes/Library/LibraryModule.h"
#include "Classes/Library/Private/ControlsFactory.h"
#include "Classes/Library/Private/LibraryData.h"
#include "Classes/Library/Private/LibraryWidget.h"

#include "Classes/Application/REGlobalOperationsData.h"
#include "Classes/Project/ProjectManagerData.h"

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
    libraryData->libraryWidget = new LibraryWidget(nullptr);
    REGlobalOperationsData* data = GetAccessor()->GetGlobalContext()->GetData<REGlobalOperationsData>();
    libraryData->libraryWidget->Init(data->GetGlobalOperations());

    DAVA::TArc::DockPanelInfo dockInfo;
    dockInfo.title = "Library";

    DAVA::TArc::PanelKey panelKey(QStringLiteral("LibraryDock"), dockInfo);
    GetUI()->AddView(REGlobal::MainWindowKey, panelKey, libraryData->libraryWidget);
    GetAccessor()->GetGlobalContext()->CreateData(std::move(libraryData));

    fieldBinder.reset(new DAVA::TArc::FieldBinder(GetAccessor()));
    DAVA::TArc::FieldDescriptor projectFieldDescriptor(DAVA::ReflectedTypeDB::Get<ProjectManagerData>(), DAVA::FastName(ProjectManagerData::ProjectPathProperty));
    fieldBinder->BindField(projectFieldDescriptor, DAVA::MakeFunction(this, &LibraryModule::OnProjectChanged));
    DAVA::TArc::FieldDescriptor libraryFieldDescriptor(DAVA::ReflectedTypeDB::Get<LibraryData>(), DAVA::FastName(LibraryData::selectedPathProperty));
    fieldBinder->BindField(libraryFieldDescriptor, DAVA::MakeFunction(this, &LibraryModule::OnSelectedPathChanged));
}

void LibraryModule::OnProjectChanged(const DAVA::Any& projectFieldValue)
{
    ProjectManagerData* projectData = GetAccessor()->GetGlobalContext()->GetData<ProjectManagerData>();
    LibraryData* libraryData = GetAccessor()->GetGlobalContext()->GetData<LibraryData>();
    if (projectData->GetProjectPath().IsEmpty())
    {
        libraryData->libraryWidget->setEnabled(false);
        libraryData->libraryWidget->SetLibraryPath(DAVA::FilePath());
    }
    else
    {
        libraryData->libraryWidget->setEnabled(true);
        libraryData->libraryWidget->SetLibraryPath(projectData->GetDataSource3DPath());
    }
}

void LibraryModule::OnSelectedPathChanged(const DAVA::Any& selectedPathValue)
{
    DAVA::FilePath selectedPath;
    if (selectedPathValue.CanGet<DAVA::FilePath>())
    {
        selectedPath = selectedPathValue.Get<DAVA::FilePath>();
    }

    if (SettingsManager::GetValue(Settings::General_PreviewEnabled).AsBool() == true)
    {
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
