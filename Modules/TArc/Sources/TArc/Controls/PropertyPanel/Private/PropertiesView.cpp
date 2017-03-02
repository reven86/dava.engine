#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Core/ContextAccessor.h"

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

#include <QHBoxLayout>
#include <QTreeView>
#include <QHeaderView>
#include <QTimer>

namespace DAVA
{
namespace TArc
{
PropertiesView::PropertiesView(UI* ui, ContextAccessor* accessor_, const FieldDescriptor& objectsField, const String& settingsNodeName_)
    : binder(accessor_)
    , accessor(accessor_)
    , settingsNodeName(settingsNodeName_)
{
    binder.BindField(objectsField, MakeFunction(this, &PropertiesView::OnObjectsChanged));
    model.reset(new ReflectedPropertyModel(ui));

    SetupUI();

    model->LoadExpanded(accessor->CreatePropertiesNode(settingsNodeName).CreateSubHolder("expandedList"));

    QTimer* timer = new QTimer(this);
    timer->setInterval(500);
    QObject::connect(timer, &QTimer::timeout, [this]()
                     {
                         model->Update();
                     });

    //timer->start();

    QObject::connect(view, &QTreeView::expanded, this, &PropertiesView::OnExpanded);
    QObject::connect(view, &QTreeView::collapsed, this, &PropertiesView::OnCollapsed);
}

PropertiesView::~PropertiesView()
{
    PropertiesItem viewSettings = accessor->CreatePropertiesNode(settingsNodeName);
    PropertiesItem item = viewSettings.CreateSubHolder("expandedList");
    model->SaveExpanded(item);
}

void PropertiesView::RegisterExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    model->RegisterExtension(extension);
}

void PropertiesView::UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    model->UnregisterExtension(extension);
}

void PropertiesView::SetupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
    layout->addWidget(view);

    view->setModel(model.get());
    view->setRootIndex(QModelIndex());
    view->setItemDelegate(new PropertiesViewDelegate(view, model.get(), this));

    QHeaderView* headerView = view->header();
    connections.AddConnection(headerView, &QHeaderView::sectionResized, MakeFunction(this, &PropertiesView::OnColumnResized));
}

void PropertiesView::OnObjectsChanged(const Any& objects)
{
    if (objects.IsEmpty())
    {
        model->SetObjects(Vector<Reflection>());
        return;
    }

    DVASSERT(objects.CanCast<Vector<Reflection>>());
    model->SetObjects(objects.Cast<Vector<Reflection>>());
    QModelIndexList expandedLIst = model->GetExpandedList();
    foreach (const QModelIndex& index, expandedLIst)
    {
        view->expand(index);
    }
}

void PropertiesView::OnColumnResized(int columnIndex, int oldSize, int newSize)
{
    PropertiesViewDelegate* d = qobject_cast<PropertiesViewDelegate*>(view->itemDelegate());
    DVASSERT(d != nullptr);
    d->UpdateSizeHints(columnIndex, newSize);
}

void PropertiesView::OnExpanded(const QModelIndex& index)
{
    //model->SetExpanded(true, index);
}

void PropertiesView::OnCollapsed(const QModelIndex& index)
{
    //model->SetExpanded(false, index);
}

} // namespace TArc
} // namespace DAVA