#pragma once

#include "Modules/LegacySupportModule/Private/Project.h"
#include "EditorSystems/SelectionContainer.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

class Project;
class Document;

class LegacySupportModule : public DAVA::TArc::ClientModule, private DAVA::TArc::DataListener
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;
    void OnContextCreated(DAVA::TArc::DataContext* context) override;

    void InitMainWindow();

    void RegisterOperations();

    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);
    void OnJumpToPrototype();
    void OnFindPrototypeInstances();

    void OnSelectionInPackageChanged(const SelectedNodes& nodes);

    DAVA::TArc::QtConnections connections;
    DAVA::TArc::DataWrapper projectDataWrapper;
    DAVA::TArc::DataWrapper documentDataWrapper;
    std::unique_ptr<Project> project;
    DAVA::Map<DAVA::TArc::DataContext::ContextID, std::unique_ptr<Document>> documents;

    DAVA_VIRTUAL_REFLECTION(LegacySupportModule, DAVA::TArc::ClientModule);
};
