#pragma once

#include "TArcCore/ConsoleModule.h"

class ConsoleCommandModule: public DAVA::TArc::ConsoleModule
{
public:
    ConsoleCommandModule(const DAVA::String& scenePath_)
        : scenePath(scenePath_)
    {
    }

protected:
    void PostInit() override;
    eFrameResult OnFrame() override;
    void BeforeDestroyed() override;

private:
    DAVA::String scenePath;
};
