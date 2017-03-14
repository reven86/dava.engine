#pragma once

#include "Model/PackageHierarchy/PackageListener.h"

#include <TArc/DataProcessing/DataNode.h>

class PackageListenerProxy : public DAVA::TArc::DataNode, public PackageListener
{
public:
    void AddListener(PackageListener* listener);
    void RemoveListener(PackageListener* listener);

private:
    friend class PackageListenerModule;
    void SetPackage(PackageNode* node);

    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property) override;

    void ControlWillBeAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;

    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;

    void StyleWillBeAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index) override;
    void StyleWasAdded(StyleSheetNode* node, StyleSheetsNode* destination, int index) override;

    void StyleWillBeRemoved(StyleSheetNode* node, StyleSheetsNode* from) override;
    void StyleWasRemoved(StyleSheetNode* node, StyleSheetsNode* from) override;

    void ImportedPackageWillBeAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;
    void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;

    void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from) override;
    void ImportedPackageWasRemoved(PackageNode* node, ImportedPackagesNode* from) override;

    void StyleSheetsWereRebuilt() override;

    PackageNode* package = nullptr;
    DAVA::List<PackageListener*> listeners;

    DAVA_VIRTUAL_REFLECTION(PackageListenerProxy, DAVA::TArc::DataNode);
};
