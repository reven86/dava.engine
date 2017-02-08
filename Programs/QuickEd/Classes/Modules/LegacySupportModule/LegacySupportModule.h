#pragma once

#include "Modules/ProjectModule/Project.h"

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

    DAVA::TArc::QtConnections connections;
    DAVA::TArc::DataWrapper projectDataWrapper;
    std::unique_ptr<Project> project;

    DAVA_VIRTUAL_REFLECTION(LegacySupportModule, DAVA::TArc::ClientModule);
};
