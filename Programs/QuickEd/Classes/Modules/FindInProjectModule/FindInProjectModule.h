#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

class FindResultsWidget;
class FindFilter;

class FindInProjectModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;

    void OnFindInProject();

    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(FindInProjectModule, DAVA::TArc::ClientModule);
};
