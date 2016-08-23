#include "LibraryModule.h"
#include "SceneViewOperations.h"

#include "TArcCore/ContextAccessor.h"
#include "WindowSubSystem/UI.h"

#include "Reflection/Registrator.h"
#include "Base/Type.h"

#include <QListWidget>
#include <QFileSystemModel>
#include <QTimer>

class FileSystemData : public tarc::DataNode
{
public:
    FileSystemData(LibraryModule* self_, QFileSystemModel* model_)
        : self(self_)
        , model(model_)
    {
    }

    ~FileSystemData()
    {
        delete model;
    }

    QFileSystemModel* GetModel() const
    {
        return model;
    }

    void OpenScene(QModelIndex index)
    {
        DAVA::String path = model->filePath(index).toStdString();
        self->InvokeOperation(SceneViewOperations::OpenScene, path);
    }

private:
    LibraryModule* self = nullptr;
    QFileSystemModel* model = nullptr;
    DAVA_VIRTUAL_REFLECTION(FileSystemData, tarc::DataNode)
    {
        DAVA::ReflectionRegistrator<FileSystemData>::Begin()
        .Field("fileSystemModel", &FileSystemData::GetModel, nullptr)
        .Method("openScene", &FileSystemData::OpenScene)
        .End();
    }
};

void LibraryModule::OnContextCreated(tarc::DataContext& context)
{
}

void LibraryModule::OnContextDeleted(tarc::DataContext& context)
{
}

void LibraryModule::PostInit()
{
    QFileSystemModel* model = new QFileSystemModel();
    model->setNameFilters(QStringList() << "*.sc2");
    model->setNameFilterDisables(false);
    model->setRootPath(QDir::rootPath());

    tarc::DataContext& globalContext = GetAccessor().GetGlobalContext();

    globalContext.CreateData(std::make_unique<FileSystemData>(this, model));

    tarc::UI& ui = GetUI();

    tarc::WindowKey windowKey(DAVA::FastName("TemplateTArc"));

    tarc::DockPanelInfo info;
    info.area = Qt::LeftDockWidgetArea;
    info.tabbed = false;
    info.title = "Library";
    ui.AddView(windowKey, tarc::PanelKey(info.title, info), "qrc:/Library.qml",
               GetAccessor().CreateWrapper(DAVA::ReflectedType::Get<FileSystemData>()));
}
