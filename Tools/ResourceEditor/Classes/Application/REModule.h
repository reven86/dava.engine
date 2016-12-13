#pragma once

#include "TArc/Core/ClientModule.h"

#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrapper.h"

class TextureCache;
class ResourceEditorLauncher;
class REModule : public DAVA::TArc::ClientModule
{
public:
    ~REModule();

protected:
    void PostInit() override;
};