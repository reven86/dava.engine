#pragma once

#include "DAVAEngine.h"
#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class GameCore;
class GPUTest : public BaseScreen
{
public:
    GPUTest(GameCore* g);

public:
    void LoadResources() override;
};
