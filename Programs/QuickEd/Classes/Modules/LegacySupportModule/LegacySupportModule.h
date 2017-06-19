#pragma once

#include "UI/Package/PackageWidget.h"
#include "Modules/LegacySupportModule/Private/Project.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

class Project;

class LegacySupportModule : public DAVA::TArc::ClientModule, private DAVA::TArc::DataListener
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void InitMainWindow();

    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);
    void OnJumpToPrototype();
    void OnFindPrototypeInstances();

    void OnSelectionInPackageChanged(const SelectedNodes& nodes);

    DAVA::TArc::QtConnections connections;
    DAVA::TArc::DataWrapper projectDataWrapper;
    DAVA::TArc::DataWrapper documentDataWrapper;
    std::unique_ptr<Project> project;
    DAVA::Map<PackageNode*, PackageContext> packageWidgetContexts;

    DAVA_VIRTUAL_REFLECTION(LegacySupportModule, DAVA::TArc::ClientModule);
};
