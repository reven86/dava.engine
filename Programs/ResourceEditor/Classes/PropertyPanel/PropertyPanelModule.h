#pragma once

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

class PropertyPanelModule final : public DAVA::TArc::ClientModule
{
public:
    void PostInit() override;

private:
    DAVA_VIRTUAL_REFLECTION(PropertyPanelModule, DAVA::TArc::ClientModule);
};