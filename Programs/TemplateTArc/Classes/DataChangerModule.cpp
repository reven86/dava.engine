#include "DataChangerModule.h"
#include "SharedData.h"

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
    ~FileSystemData()
    {
        delete model;
    }

    QFileSystemModel* GetModel() const
    {
        return model;
    }

    QFileSystemModel* model = nullptr;
    DAVA::String sampleText;

private:
    DAVA_VIRTUAL_REFLECTION(FileSystemData, tarc::DataNode)
    {
        DAVA::ReflectionRegistrator<FileSystemData>::Begin()
        // TODO check with s_zdanevich
        //.Field("fileSystemModel", &FileSystemData::model)
        .Field("fileSystemModel", &FileSystemData::GetModel, nullptr)
        .Field("sampleText", &FileSystemData::sampleText)
        .End();
    }
};

void DataChangerModule::OnContextCreated(tarc::DataContext& context)
{
    std::unique_ptr<FileSystemData> data = std::make_unique<FileSystemData>();
    data->model = new QFileSystemModel();
    data->model->setNameFilters(QStringList() << "*.sc2");
    data->model->setNameFilterDisables(false);
    data->model->setRootPath(QDir::rootPath());
    data->sampleText = "Hello cruel World";

    context.CreateData(std::move(data));
}

void DataChangerModule::OnContextDeleted(tarc::DataContext& context)
{
}

void DataChangerModule::PostInit()
{
    tarc::UI& ui = GetUI();
    wrapper = GetAccessor().CreateWrapper(DAVA::ReflectedType::Get<SharedData>());
    wrapper.AddListener(this);

    QListWidget* customerList = new QListWidget();
    customerList->addItems(QStringList()
                           << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
                           << "Jane Doe, Memorabilia, 23 Watersedge, Beaton"
                           << "Tammy Shea, Tiblanka, 38 Sea Views, Carlton"
                           << "Tim Sheen, Caraba Gifts, 48 Ocean Way, Deal"
                           << "Sol Harvey, Chicos Coffee, 53 New Springs, Eccleston"
                           << "Sally Hobart, Tiroli Tea, 67 Long River, Fedula");

    tarc::WindowKey windowKey(DAVA::FastName("TemplateTArc"));

    tarc::DockPanelInfo info;
    info.title = "Customers";
    ui.AddView(windowKey, tarc::PanelKey(info.title, info), customerList);

    QListWidget* paragraphs = new QListWidget();
    paragraphs->addItems(QStringList()
                         << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
                         << "Jane Doe, Memorabilia, 23 Watersedge, Beaton"
                         << "Tammy Shea, Tiblanka, 38 Sea Views, Carlton"
                         << "Tim Sheen, Caraba Gifts, 48 Ocean Way, Deal"
                         << "Sol Harvey, Chicos Coffee, 53 New Springs, Eccleston"
                         << "Sally Hobart, Tiroli Tea, 67 Long River, Fedula");

    info.title = "Paragraphs";
    ui.AddView(windowKey, tarc::PanelKey(info.title, info), paragraphs);

    info.area = Qt::LeftDockWidgetArea;
    info.tabbed = false;
    info.title = "Library";
    ui.AddView(windowKey, tarc::PanelKey(info.title, info), "qrc:/Library.qml",
               GetAccessor().CreateWrapper(DAVA::ReflectedType::Get<FileSystemData>()));

    QTimer::singleShot(5000, [this]()
                       {
                           GetAccessor().GetActiveContext().GetData<FileSystemData>().sampleText = "new text";
                       });
}

void DataChangerModule::OnDataChanged(const tarc::DataWrapper& dataWrapper, const DAVA::Set<DAVA::String>& fields)
{
    if (wrapper.HasData())
    {
        tarc::DataEditor<SharedData> editor = wrapper.CreateEditor<SharedData>();
        DAVA::Logger::Info("Changer %d", editor->GetValue());
        editor->SetValue(editor->GetValue() + 1);
    }
    else
    {
        DAVA::Logger::Info("Changer empty");
    }
}
