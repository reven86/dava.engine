#include "DataChangerModule.h"
#include "SharedData.h"

#include "TArcCore/ContextAccessor.h"
#include "WindowSubSystem/UI.h"

#include "Base/Type.h"

#include <QListWidget>

void DataChangerModule::OnContextCreated(tarc::DataContext& context)
{
}

void DataChangerModule::OnContextDeleted(tarc::DataContext& context)
{
}

void DataChangerModule::PostInit(tarc::UI& ui)
{
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

    tarc::DockPanelInfo info;
    info.tittle = "Customers";
    ui.AddView(tarc::WindowKey(DAVA::FastName("TemplateTArc"), info), customerList);

    QListWidget* paragraphs = new QListWidget();
    paragraphs->addItems(QStringList()
                         << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
                         << "Jane Doe, Memorabilia, 23 Watersedge, Beaton"
                         << "Tammy Shea, Tiblanka, 38 Sea Views, Carlton"
                         << "Tim Sheen, Caraba Gifts, 48 Ocean Way, Deal"
                         << "Sol Harvey, Chicos Coffee, 53 New Springs, Eccleston"
                         << "Sally Hobart, Tiroli Tea, 67 Long River, Fedula");

    info.tittle = "Paragraphs";
    ui.AddView(tarc::WindowKey(DAVA::FastName("TemplateTArc"), info), paragraphs);
}

void DataChangerModule::OnDataChanged(const tarc::DataWrapper& wrapper_)
{
    if (wrapper.HasData())
    {
        tarc::DataWrapper::Editor<SharedData> editor = wrapper.CreateEditor<SharedData>();
        DAVA::Logger::Info("Changer %d", editor->GetValue());
        editor->SetValue(editor->GetValue() + 1);
    }
    else
    {
        DAVA::Logger::Info("Changer empty");
    }
}
