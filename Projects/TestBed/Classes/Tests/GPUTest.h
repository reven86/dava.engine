#pragma once

#include "DAVAEngine.h"
#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class GameCore;
class GPUTest : public BaseScreen
{
public:
    GPUTest(GameCore& gameCore);

public:
    void LoadResources() override;
};
