#include "Modules/FileSystemModule/FileSystemModule.h"
#include "Modules/FileSystemModule/FileSystemWidget.h"
#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(FileSystemModule)
{
    DAVA::ReflectionRegistrator<FileSystemModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FileSystemModule::PostInit()
{
    InitUI();
    RegisterOperations();
}

void FileSystemModule::InitUI()
{
    using namespace DAVA::TArc;

    const char* title = "File System";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::LeftDockWidgetArea;
    PanelKey panelKey(title, panelInfo);

    widget = new FileSystemWidget(GetAccessor());
    widget->openFile.Connect(this, &FileSystemModule::OnOpenFile);
    GetUI()->AddView(DAVA::TArc::mainWindowKey, panelKey, widget);
}

void FileSystemModule::RegisterOperations()
{
    RegisterOperation(QEGlobal::SelectFile.ID, widget, &FileSystemWidget::SelectFile);
}

void FileSystemModule::OnOpenFile(const QString& filePath)
{
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, filePath);
}

DECL_GUI_MODULE(FileSystemModule);
