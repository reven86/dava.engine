#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

class SelectionModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void PostInit() override;

private:
    void CreateModuleActions(DAVA::TArc::UI* ui);
    void SelectionByMouseChanged();

    DAVA::TArc::QtConnections connections;
};
