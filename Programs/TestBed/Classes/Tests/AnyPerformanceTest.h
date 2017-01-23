#pragma once

#include "Infrastructure/BaseScreen.h"

class TestBed;
class AnyPerformanceTest : public BaseScreen
{
public:
    AnyPerformanceTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

    UITextField* testCount;
    UIStaticText* resultCreate;
    UIStaticText* resultGetSet;

    uint64 GetLoopCount();
    void SetResult(UIStaticText*, uint64 ms);
    void OnCreateTest(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnGetSetTest(DAVA::BaseObject* sender, void* data, void* callerData);
};
