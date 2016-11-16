#pragma once

#include "TArc/Core/ClientModule.h"

class InitModule : public DAVA::TArc::ClientModule
{
public:
    ~InitModule();

protected:
    void PostInit() override;
    void UnpackHelpDoc();
};