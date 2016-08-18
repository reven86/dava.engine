#include "LibraryModule.h"

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
    FileSystemData(QFileSystemModel* model_)
        : model(model_)
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
        int x = 0;
        x++;
    }

private:
    QFileSystemModel* model = nullptr;
    DAVA_VIRTUAL_REFLECTION(FileSystemData, tarc::DataNode)
    {
        DAVA::ReflectionRegistrator<FileSystemData>::Begin()
        // TODO check with s_zdanevich
        //.Field("fileSystemModel", &FileSystemData::model)
        .Field("fileSystemModel", &FileSystemData::GetModel, nullptr)
        .Method("openScene", &FileSystemData::OpenScene)
        .End();
    }
};

void LibraryModule::OnContextCreated(tarc::DataContext& context)
{
    QFileSystemModel* model = new QFileSystemModel();
    model->setNameFilters(QStringList() << "*.sc2");
    model->setNameFilterDisables(false);
    model->setRootPath(QDir::rootPath());

    context.CreateData(std::make_unique<FileSystemData>(model));
}

void LibraryModule::OnContextDeleted(tarc::DataContext& context)
{
    context.DeleteData<FileSystemData>();
}

void LibraryModule::PostInit()
{
    tarc::UI& ui = GetUI();

    tarc::WindowKey windowKey(DAVA::FastName("TemplateTArc"));

    tarc::DockPanelInfo info;
    info.area = Qt::LeftDockWidgetArea;
    info.tabbed = false;
    info.title = "Library";
    ui.AddView(windowKey, tarc::PanelKey(info.title, info), "qrc:/Library.qml",
               GetAccessor().CreateWrapper(DAVA::ReflectedType::Get<FileSystemData>()));
}
