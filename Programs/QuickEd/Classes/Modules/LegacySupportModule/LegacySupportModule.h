#pragma once

#include "Modules/LegacySupportModule/Private/Project.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

class Project;

class LegacySupportModule : public DAVA::TArc::ClientModule, private DAVA::TArc::DataListener
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key);
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne);

    void InitMainWindow();

    void RegisterOperations();

    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);
    void OnJumpToPrototype();

    DAVA::TArc::QtConnections connections;
    DAVA::TArc::DataWrapper projectDataWrapper;
    std::unique_ptr<Project> project;
    DAVA::Map<DAVA::TArc::DataContext::ContextID, std::unique_ptr<Document>> documents;

    DAVA_VIRTUAL_REFLECTION(LegacySupportModule, DAVA::TArc::ClientModule);
};
