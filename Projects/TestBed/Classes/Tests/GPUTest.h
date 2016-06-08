#pragma once

#include "DAVAEngine.h"
#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class GPUTest : public BaseScreen
{
public:
    GPUTest();

public:
    void LoadResources() override;
};
