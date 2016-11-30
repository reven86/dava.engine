#pragma once

#include "TArc/Core/ClientModule.h"

class InitModule : public DAVA::TArc::ClientModule
{
public:
    ~InitModule();

protected:
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void PostInit() override;
};