#ifndef __CORETEST_TEST_H__
#define __CORETEST_TEST_H__

#include <DAVAEngine.h>
#include "Infrastructure/BaseScreen.h"

class CoreTest : public BaseScreen
{
public:
    CoreTest();

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    void Quit(BaseObject* obj, void* data, void* callerData);
};

#endif //__CORETEST_TEST_H__
