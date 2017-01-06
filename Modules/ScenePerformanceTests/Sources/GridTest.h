#pragma once

#include <Base/BaseTypes.h>
#include <Base/ScopedPtr.h>
#include <Math/Vector.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/File.h>
#include <Scene3D/Scene.h>
#include <UI/UI3DView.h>
#include <Engine/Engine.h>
#include <Functional/SignalBase.h>
#include <Utils/FpsMeter.h>

namespace DAVA
{
    class Texture;
}

class GridTestListener
{
public:
    virtual void OnGridTestStateChanged() = 0;
};

class GridTest
{
public:
    enum State
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
    struct Sample
    {
        DAVA::Vector3 pos;
        DAVA::float32 angle = 0.0f;
        DAVA::float32 sine = 0.0f;
        DAVA::float32 cos = 0.0f;

        DAVA::float32 fps = 0.0f;
    };

    class ScreenshotSaver
    {
    public:
        explicit ScreenshotSaver(DAVA::FilePath& path, Sample& sample);
        void SaveTexture(DAVA::Texture* screenshot);

        Sample sample;
        DAVA::FilePath savePath;
        bool saved = false;
    };

    void Update(DAVA::float32 timeElapsed);
    void MakeScreenshot(ScreenshotSaver&);
    void SetSamplePosition(Sample& sample);
    void ProcessMeasuredSamples();
    void RemoveSavedScreenshots();

    void SetState(State newState);

private:
    DAVA::Engine& engine;
    DAVA::SigConnectionID updateSignalID = 0;

    GridTestListener* listener = nullptr;
    DAVA::Scene* scene = nullptr;
    DAVA::ScopedPtr<DAVA::UI3DView> sceneView;

    State state = State::Finished;

    FpsMeter fpsMeter;

    DAVA::uint32 sampleIndex = 0;
    DAVA::Vector<Sample> samples;
    DAVA::List<ScreenshotSaver> screenshotsToStart;
    DAVA::List<ScreenshotSaver> screenshotsToSave;
    bool isEvenFrame = false;
    DAVA::FilePath reportFolderPath;
    DAVA::ScopedPtr<DAVA::File> reportFile;
};

inline GridTest::State GridTest::GetState() const
{
    return state;
}
