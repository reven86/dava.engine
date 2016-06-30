#ifndef __CORETEST_TEST_H__
#define __CORETEST_TEST_H__

#include <DAVAEngine.h>
#include <Engine/EngineFwd.h>

#include "Infrastructure/BaseScreen.h"

class GameCore;
class CoreTest : public BaseScreen
{
public:
    CoreTest(GameCore* g);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void Quit(BaseObject* obj, void* data, void* callerData);

private:
    DAVA::Engine* engine = nullptr;
};

#endif //__CORETEST_TEST_H__
