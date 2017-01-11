#pragma once

#include <Base/BaseTypes.h>
#include <Base/ScopedPtr.h>
#include <Engine/Engine.h>
#include <UI/UI3DView.h>

class GridTestListener
{
public:
    virtual void OnGridTestStateChanged() = 0;
};

class GridTestImpl;

class GridTest final
{
public:
    enum State : DAVA::uint8
    {
        Running,
        MakingScreenshots,
        Finished
    };

    explicit GridTest(DAVA::Engine& engine, GridTestListener* listener);
    ~GridTest();

    bool Start(const DAVA::ScopedPtr<DAVA::UI3DView>& s);
    void Stop();

    State GetState() const;

private:
    GridTestImpl* impl = nullptr;
};
