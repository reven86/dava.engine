#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"

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
PropertiesView::PropertiesView(ContextAccessor* accessor, const FieldDescriptor& objectsField, const std::weak_ptr<Updater>& updater_)
    : binder(accessor)
    , updater(updater_)
{
    binder.BindField(objectsField, MakeFunction(this, &PropertiesView::OnObjectsChanged));
    model.reset(new ReflectedPropertyModel());

    SetupUI();

    std::shared_ptr<Updater> lockedUpdater = updater.lock();
    if (lockedUpdater != nullptr)
    {
        updateConnectionID = lockedUpdater->update.Connect(this, &PropertiesView::Update);
    }
}

PropertiesView::~PropertiesView()
{
    std::shared_ptr<Updater> lockedUpdater = updater.lock();
    if (lockedUpdater != nullptr)
    {
        lockedUpdater->update.Disconnect(updateConnectionID);
    }
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
}

void PropertiesView::OnColumnResized(int columnIndex, int oldSize, int newSize)
{
    PropertiesViewDelegate* d = qobject_cast<PropertiesViewDelegate*>(view->itemDelegate());
    DVASSERT(d != nullptr);
    d->UpdateSizeHints(columnIndex, newSize);
}

void PropertiesView::Update(UpdatePolicy policy)
{
    switch (policy)
    {
    case DAVA::TArc::PropertiesView::FullUpdate:
        model->Update();
        break;
    case DAVA::TArc::PropertiesView::FastUpdate:
        model->UpdateFast();
        break;
    default:
        DVASSERT(false, "Unimplemented update policy have been received");
        break;
    }
}
}
}