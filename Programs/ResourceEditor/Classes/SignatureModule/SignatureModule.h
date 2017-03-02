#pragma once

#include <TArc/Core/ClientModule.h>
#include <Reflection/Reflection.h>

class SignatureModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void PostInit() override;

private:
    DAVA::String currentUserName;
    DAVA_VIRTUAL_REFLECTION(SignatureModule, DAVA::TArc::ClientModule);
};
