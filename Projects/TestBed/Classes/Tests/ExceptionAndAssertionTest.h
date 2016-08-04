#pragma once
#include <DAVAEngine.h>
#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class ExceptionAndAssertionTest : public BaseScreen
{
public:
    ExceptionAndAssertionTest();

protected:
    void LoadResources() override;
    void UnloadResources() override;
};
