#include "Modules/PackageListenerModule/PackageListenerProxy.h"

#include "Model/PackageHierarchy/PackageNode.h"

DAVA_VIRTUAL_REFLECTION_IMPL(PackageListenerProxy)
{
    DAVA::ReflectionRegistrator<PackageListenerProxy>::Begin()
    .ConstructorByPointer()
    .End();
}

void PackageListenerProxy::AddListener(PackageListener* listener)
{
    if (std::find(listeners.begin(), listeners.end(), listener) != listeners.end())
    {
        DVASSERT(false, "can not add listener which is already added to packageListenerProxy");
        return;
    }
    listeners.push_back(listener);
}

void PackageListenerProxy::RemoveListener(PackageListener* listener)
{
    if (std::find(listeners.begin(), listeners.end(), listener) == listeners.end())
    {
        DVASSERT(false, "can not remove not-added listener from packageListenerProxy");
        return;
    }
    listeners.remove(listener);
}

void PackageListenerProxy::SetPackage(PackageNode* node)
{
    if (package != nullptr)
    {
        package->RemoveListener(this);
    }
    package = node;
    if (package != nullptr)
    {
        package->AddListener(this);
    }
}

void PackageListenerProxy::PackageNodeWasChanged(PackageNode* node)
{
    for (PackageListener* listener : listeners)
    {
        listener->PackageNodeWasChanged(node);
    }
}

void PackageListenerProxy::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    for (PackageListener* listener : listeners)
    {
        listener->ControlPropertyWasChanged(node, property);
    }
}

void PackageListenerProxy::StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property)
{
    for (PackageListener* listener : listeners)
    {
        listener->StylePropertyWasChanged(node, property);
    }
}

void PackageListenerProxy::ControlWillBeAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    for (PackageListener* listener : listeners)
    {
        listener->ControlWillBeAdded(node, destination, index);
    }
}

void PackageListenerProxy::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    for (PackageListener* listener : listeners)
    {
        listener->ControlWasAdded(node, destination, index);
    }
}

void PackageListenerProxy::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    for (PackageListener* listener : listeners)
    {
        listener->ControlWillBeRemoved(node, from);
    }
}

void PackageListenerProxy::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    for (PackageListener* listener : listeners)
    {
        listener->ControlWasRemoved(node, from);
    }
}

void PackageListenerProxy::StyleWillBeAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index)
{
    for (PackageListener* listener : listeners)
    {
        listener->StyleWillBeAdded(node, destination, index);
    }
}

void PackageListenerProxy::StyleWasAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index)
{
    for (PackageListener* listener : listeners)
    {
        listener->StyleWasAdded(node, destination, index);
    }
}

void PackageListenerProxy::StyleWillBeRemoved(StyleSheetNode* node, StyleSheetsNode* from)
{
    for (PackageListener* listener : listeners)
    {
        listener->StyleWillBeRemoved(node, from);
    }
}

void PackageListenerProxy::StyleWasRemoved(StyleSheetNode* node, StyleSheetsNode* from)
{
    for (PackageListener* listener : listeners)
    {
        listener->StyleWasRemoved(node, from);
    }
}

void PackageListenerProxy::ImportedPackageWillBeAdded(PackageNode* node, ImportedPackagesNode* to, int index)
{
    for (PackageListener* listener : listeners)
    {
        listener->ImportedPackageWillBeAdded(node, to, index);
    }
}

void PackageListenerProxy::ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index)
{
    for (PackageListener* listener : listeners)
    {
        listener->ImportedPackageWasAdded(node, to, index);
    }
}

void PackageListenerProxy::ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    for (PackageListener* listener : listeners)
    {
        listener->ImportedPackageWillBeRemoved(node, from);
    }
}

void PackageListenerProxy::ImportedPackageWasRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    for (PackageListener* listener : listeners)
    {
        listener->ImportedPackageWasRemoved(node, from);
    }
}

void PackageListenerProxy::StyleSheetsWereRebuilt()
{
    for (PackageListener* listener : listeners)
    {
        listener->StyleSheetsWereRebuilt();
    }
}
