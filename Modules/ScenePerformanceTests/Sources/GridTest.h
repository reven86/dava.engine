#pragma once

#include <Base/BaseTypes.h>
#include <Base/ScopedPtr.h>
#include <Engine/Engine.h>
#include <UI/UI3DView.h>

class GridTestListener
{
public:
    virtual ~GridTestListener() {}
    virtual void OnGridTestStateChanged() = 0;
};

class GridTestImpl;

struct GridTestSample
{
    DAVA::Vector3 pos;
    DAVA::float32 angle = 0.0f;
    DAVA::float32 sine = 0.0f;
    DAVA::float32 cos = 0.0f;
    DAVA::float32 fps = 0.0f;
    DAVA::FilePath screenshotPath;
};

struct GridTestResult
{
    DAVA::float32 sceneSize = 0.f;
    DAVA::float32 sceneMin = 0.f;
    DAVA::float32 sceneMax = 0.f;
    DAVA::float32 gridStep = 0.f;
    DAVA::float32 sampleAngleDegrees = 0.f;
    DAVA::FilePath panoramaPath;
    DAVA::Vector<GridTestSample> samples;
    DAVA::float32 avgFPS = 0.f;
    DAVA::float32 minFPS = 0.f;
    DAVA::float32 maxFPS = 0.f;
};

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
    GridTestResult& GetResult();

private:
    GridTestImpl* impl = nullptr;
};
