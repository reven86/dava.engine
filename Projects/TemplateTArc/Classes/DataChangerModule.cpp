#include "DataChangerModule.h"
#include "SharedData.h"

#include "TArcCore/ContextAccessor.h"
#include "WindowSubSystem/UI.h"

#include "Base/Type.h"

#include <QListWidget>
#include <QFileSystemModel>
#include <QTimer>

#include <thread>

class FileSystemData : public tarc::DataNode
{
    DAVA_DECLARE_TYPE_INITIALIZER
    DAVA_DECLARE_TYPE_VIRTUAL_REFLECTION;

public:
    ~FileSystemData()
    {
        delete model;
    }

    IMPLEMENT_TYPE(FileSystemData);

    QFileSystemModel* model = nullptr;
    DAVA::String sampleText;
};

DAVA_TYPE_INITIALIZER(FileSystemData)
{
    DAVA::ReflectionRegistrator<FileSystemData>::Begin()
    .Base<DataNode>()
    .Field("fileSystemModel", &FileSystemData::model)
    .Field("sampleText", &FileSystemData::sampleText)
    .End();
}

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
    wrapper = GetAccessor().CreateWrapper(DAVA::Type::Instance<SharedData>());
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
               GetAccessor().CreateWrapper(DAVA::Type::Instance<FileSystemData>()));
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
