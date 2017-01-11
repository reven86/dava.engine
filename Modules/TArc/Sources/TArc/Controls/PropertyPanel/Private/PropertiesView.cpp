#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/Controls/PropertyPanel/ReflectedPropertyModel.h"

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

#include <QHBoxLayout>
#include <QTreeView>

namespace DAVA
{
namespace TArc
{
PropertiesView::PropertiesView(ContextAccessor* accessor, const FieldDescriptor& objectsField)
    : binder(accessor)
{
    binder.BindField(objectsField, MakeFunction(this, &PropertiesView::OnObjectsChanged));
    model.reset(new ReflectedPropertyModel());

    SetupUI();
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
    layout->addWidget(view);

    view->setModel(model.get());
    view->setRootIndex(QModelIndex());
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
}
}