#include "GridTest.h"

#include <FileSystem/FileSystem.h>
#include <Render/Texture.h>
#include <Render/Image/Image.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/Camera.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Platform/DateTime.h>
#include <UI/UIControlSystem.h>
#include <UI/UIScreenshoter.h>
#include <Logger/Logger.h>
#include <Utils/StringFormat.h>
#include <Math/Vector.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/File.h>
#include <Scene3D/Scene.h>
#include <Functional/SignalBase.h>
#include <Utils/FpsMeter.h>

namespace GridTestDetails
{
const DAVA::uint32 GRID_SIZE = 8;
const DAVA::uint32 ANGLE_COUNT = 8;
const DAVA::float32 EXPOSURE_DURATION_SEC = 3.f;
const DAVA::float32 THRESHOLD_LOW_FPS = 30.0f;
const DAVA::float32 ABOVE_LANDSCAPE_ELEVATION = 10.f;

const DAVA::float32 ANGLE_STEP_DEGREES = 360.f / ANGLE_COUNT;

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

ScreenshotSaver::ScreenshotSaver(DAVA::FilePath& path, Sample& sample)
    : savePath(path)
    , sample(sample)
{
}

void ScreenshotSaver::SaveTexture(DAVA::Texture* screenshot)
{
    using namespace DAVA;
    ScopedPtr<Image> image(screenshot->CreateImageFromMemory());
    const Size2i& size = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();
    image->ResizeCanvas(static_cast<uint32>(size.dx), static_cast<uint32>(size.dy));
    image->Save(savePath);
    saved = true;
}
}

class GridTestImpl final
{
public:
    explicit GridTestImpl(DAVA::Engine& engine, GridTestListener* listener);
    ~GridTestImpl();

    bool Start(const DAVA::ScopedPtr<DAVA::UI3DView>& s);
    void Stop();

    GridTest::State GetState() const;

private:
    void Update(DAVA::float32 timeElapsed);
    void MakeScreenshot(GridTestDetails::ScreenshotSaver&);
    void SetSamplePosition(GridTestDetails::Sample& sample);
    void ProcessMeasuredSamples();
    void RemoveSavedScreenshots();

    void SetState(GridTest::State newState);

private:
    DAVA::Engine& engine;
    DAVA::SigConnectionID updateSignalID = 0;

    GridTestListener* listener = nullptr;
    DAVA::Scene* scene = nullptr;
    DAVA::ScopedPtr<DAVA::UI3DView> sceneView;

    GridTest::State state = GridTest::State::Finished;

    FpsMeter fpsMeter;

    DAVA::uint32 sampleIndex = 0;
    DAVA::Vector<GridTestDetails::Sample> samples;
    DAVA::List<GridTestDetails::ScreenshotSaver> screenshotsToStart;
    DAVA::List<GridTestDetails::ScreenshotSaver> screenshotsToSave;
    DAVA::uint32 framesSinceLastSave = 0;
    DAVA::FilePath reportFolderPath;
    DAVA::ScopedPtr<DAVA::File> reportFile;
};

GridTest::State GridTestImpl::GetState() const
{
    return state;
}

GridTestImpl::GridTestImpl(DAVA::Engine& engine, GridTestListener* listener)
    : engine(engine)
    , listener(listener)
    , fpsMeter(GridTestDetails::EXPOSURE_DURATION_SEC)
{
    updateSignalID = engine.update.Connect(this, &GridTestImpl::Update);
}

GridTestImpl::~GridTestImpl()
{
    engine.update.Disconnect(updateSignalID);
    Stop();
}

bool GridTestImpl::Start(const DAVA::ScopedPtr<DAVA::UI3DView>& view)
{
    using namespace GridTestDetails;
    using namespace DAVA;

    if (state != GridTest::State::Finished)
    {
        DVASSERT(false, "can't start already started test");
        return false;
    }

    sceneView = view;
    if (!sceneView)
    {
        DVASSERT(false, "scene view is empty");
        return false;
    }

    scene = sceneView->GetScene();
    if (!scene)
    {
        DVASSERT(false, "scene view contains no scene");
        return false;
    }

    DAVA::Landscape* landscape = FindLandscape(scene);
    if (!landscape)
    {
        Logger::Warning("Grid test needs landscape in scene to be started");
        return false;
    }

    samples.reserve(GRID_SIZE * GRID_SIZE * ANGLE_COUNT);

    float32 landscapeSize = landscape->GetLandscapeSize();

    float32 step = landscapeSize / (GRID_SIZE + 1);
    float32 xMin = -landscapeSize / 2 + step;
    float32 xMax = landscapeSize / 2 - step;
    float32 yMin = -landscapeSize / 2 + step;

    bool invertedDirection = false;
    float32 yPos = yMin;
    for (uint32 y = 0; y < GRID_SIZE; ++y, yPos += step)
    {
        float32 xPos = invertedDirection ? xMax : xMin;
        float32 xInc = invertedDirection ? -step : step;
        for (uint32 x = 0; x < GRID_SIZE; ++x, xPos += xInc)
        {
            float32 angle = 0.1f;
            for (uint32 n = 0; n < ANGLE_COUNT; ++n, angle += ANGLE_STEP_DEGREES)
            {
                samples.push_back(Sample());
                Sample& testPosition = samples.back();

                testPosition.pos.x = xPos;
                testPosition.pos.y = yPos;

                float32 landscapeHeight = 0.0;
                landscape->GetHeightAtPoint(testPosition.pos, landscapeHeight);
                testPosition.pos.z = landscapeHeight + ABOVE_LANDSCAPE_ELEVATION;

                testPosition.angle = angle;
                SinCosFast(DegToRad(angle), testPosition.sine, testPosition.cos);
            }
        }

        invertedDirection = !invertedDirection;
    }

    SetState(GridTest::State::Running);
    DateTime now = DateTime::Now();
    reportFolderPath = FilePath(Format("~doc:/PerformanceReports/Report_%u/", now.GetTimestamp()));
    FileSystem::Instance()->CreateDirectory(reportFolderPath, true);
    reportFile = File::Create(reportFolderPath + "report.txt", File::CREATE | File::WRITE);

    framesSinceLastSave = 0;
    screenshotsToStart.clear();
    screenshotsToSave.clear();

    sampleIndex = 0;
    SetSamplePosition(samples[sampleIndex]);

    return true;
}

void GridTestImpl::Stop()
{
    if (state == GridTest::State::Running)
    {
        DAVA::FileSystem::Instance()->DeleteDirectory(reportFolderPath);
        state = GridTest::State::Finished;
    }
    else if (state == GridTest::State::MakingScreenshots)
    {
        screenshotsToStart.clear();
        // as soon as currently processing screenshots will be saved, class state changes to Finished
    }
}

void GridTestImpl::SetSamplePosition(GridTestDetails::Sample& sample)
{
    DAVA::Camera* camera = scene->GetCurrentCamera();
    camera->SetPosition(DAVA::Vector3(sample.pos.x, sample.pos.y, sample.pos.z));
    camera->SetDirection(DAVA::Vector3(sample.cos, sample.sine, 0));
}

void GridTestImpl::MakeScreenshot(GridTestDetails::ScreenshotSaver& screenshotSaver)
{
    SetSamplePosition(screenshotSaver.sample);
    DAVA::UIControlSystem::Instance()->GetScreenshoter()->MakeScreenshot(sceneView, DAVA::FORMAT_RGBA8888, MakeFunction(&screenshotSaver, &GridTestDetails::ScreenshotSaver::SaveTexture));
}

void GridTestImpl::RemoveSavedScreenshots()
{
    auto it = screenshotsToSave.begin();
    while (it != screenshotsToSave.end())
    {
        GridTestDetails::ScreenshotSaver& screenshotSaver = *it;
        if (screenshotSaver.saved)
        {
            auto itDel = it++;
            screenshotsToSave.erase(itDel);
        }
        else
        {
            ++it;
        }
    }
}

void GridTestImpl::ProcessMeasuredSamples()
{
    using namespace DAVA;

    if (samples.empty())
    {
        DVASSERT(false);
        return;
    }

    float32 avgFps = 0.0;
    float32 minFps = 60.0;
    float32 maxFps = 0.0;
    for (uint32 sampleIndex = 0; sampleIndex < samples.size(); ++sampleIndex)
    {
        GridTestDetails::Sample& sample = samples[sampleIndex];

        reportFile->WriteLine(Format("Sample %.0f.%.0f.%.0f angle %.0f-%.0f: fps %.1f", sample.pos.x, sample.pos.y, sample.pos.z, sample.cos, sample.sine, sample.fps));

        avgFps += sample.fps;

        if (sample.fps < minFps)
            minFps = sample.fps;

        if (sample.fps > maxFps)
            maxFps = sample.fps;

        if (sample.fps < GridTestDetails::THRESHOLD_LOW_FPS)
        {
            String screenshotName = Format("screenshot_fps%2.0f_frame%u.png", sample.fps, sampleIndex);
            FilePath path = reportFolderPath + screenshotName;
            screenshotsToStart.emplace_back(GridTestDetails::ScreenshotSaver(path, sample));
        }
    }
    avgFps /= samples.size();
    String total = Format("Avg fps: %.1f, min %.1f, max %.1f", avgFps, minFps, maxFps);
    Logger::Info("%s", total.c_str());
    reportFile->WriteLine(total);
    reportFile.reset();

    SetState(GridTest::State::MakingScreenshots);
}

void GridTestImpl::Update(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    switch (state)
    {
    case GridTest::State::Running:
    {
        fpsMeter.Update(timeElapsed);
        if (fpsMeter.IsFpsReady())
        {
            GridTestDetails::Sample& sample = samples[sampleIndex];
            sample.fps = fpsMeter.GetFps();

            if (++sampleIndex < samples.size())
            {
                SetSamplePosition(samples[sampleIndex]);
            }
            else
            {
                ProcessMeasuredSamples();
            }
        }
        return;
    }
    case GridTest::State::MakingScreenshots:
    {
        if (!screenshotsToStart.empty())
        {
            if (framesSinceLastSave++ == 5) // making screenshots only on every 5th frame It's a hack to avoid out of memory errors.
            {
                framesSinceLastSave = 0;

                // move first element from screenshotsToStart to the end of screenshotsToSave
                screenshotsToSave.splice(screenshotsToSave.end(), screenshotsToStart, screenshotsToStart.begin());

                MakeScreenshot(screenshotsToSave.back());
            }
        }
        else
        {
            RemoveSavedScreenshots();

            if (screenshotsToSave.empty())
            {
                SetState(GridTest::State::Finished);
            }
        }

        return;
    }
    case GridTest::State::Finished:
    default:
    {
        return;
    }
    }
}

void GridTestImpl::SetState(GridTest::State newState)
{
    GridTest::State prevState = state;
    state = newState;

    if (listener && newState != prevState)
    {
        listener->OnGridTestStateChanged();
    }
}

GridTest::GridTest(DAVA::Engine& engine, GridTestListener* listener)
    : impl(new GridTestImpl(engine, listener))
{
}

GridTest::~GridTest()
{
    DAVA::SafeDelete(impl);
}

bool GridTest::Start(const DAVA::ScopedPtr<DAVA::UI3DView>& s)
{
    return impl->Start(s);
}

void GridTest::Stop()
{
    impl->Stop();
}

GridTest::State GridTest::GetState() const
{
    return impl->GetState();
}
