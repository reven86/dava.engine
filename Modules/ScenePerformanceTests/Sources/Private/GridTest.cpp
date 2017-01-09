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

GridTest::ScreenshotSaver::ScreenshotSaver(DAVA::FilePath& path, Sample& sample)
    : savePath(path)
    , sample(sample)
{
}

void GridTest::ScreenshotSaver::SaveTexture(DAVA::Texture* screenshot)
{
    using namespace DAVA;
    ScopedPtr<Image> image(screenshot->CreateImageFromMemory());
    const Size2i& size = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();
    image->ResizeCanvas(static_cast<uint32>(size.dx), static_cast<uint32>(size.dy));
    image->Save(savePath);
    saved = true;
}

namespace GridTestDetails
{
    const DAVA::uint32 GRID_SIZE = 4;
    const DAVA::uint32 ANGLE_COUNT = 4;
    const DAVA::float32 EXPOSURE_DURATION_SEC = 0.1f;
    const DAVA::float32 THRESHOLD_LOW_FPS = 57.0f;
}

GridTest::GridTest(DAVA::Engine& engine, GridTestListener* listener)
    : engine(engine)
    , listener(listener)
    , fpsMeter(GridTestDetails::EXPOSURE_DURATION_SEC)
{
    updateSignalID = engine.update.Connect(this, &GridTest::Update);
}

GridTest::~GridTest()
{
    engine.update.Disconnect(updateSignalID);
    Stop();
}

bool GridTest::Start(const DAVA::ScopedPtr<DAVA::UI3DView>& view)
{
    using namespace GridTestDetails;
    using namespace DAVA;

    if (state != State::Finished)
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
    float32 angleStep = 360.f / ANGLE_COUNT;

    bool invertedDirection = false;
    float32 yPos = yMin;
    for (uint32 y = 0; y < GRID_SIZE; ++y, yPos += step)
    {
        float32 xPos = invertedDirection ? xMax : xMin;
        float32 xInc = invertedDirection ? -step : step;
        for (uint32 x = 0; x < GRID_SIZE; ++x, xPos += xInc)
        {
            static float32 angle = 0.1f;
            for (uint32 n = 0; n < ANGLE_COUNT; ++n, angle += angleStep)
            {
                samples.push_back(Sample());
                Sample& testPosition = samples.back();

                testPosition.pos.x = xPos;
                testPosition.pos.y = yPos;

                float landscapeHeight = 0.0;
                landscape->GetHeightAtPoint(testPosition.pos, landscapeHeight);
                testPosition.pos.z = landscapeHeight + 10.0f;

                testPosition.angle = angle;
                SinCosFast(DegToRad(angle), testPosition.sine, testPosition.cos);
            }
        }

        invertedDirection = !invertedDirection;
    }

    SetState(State::Running);
    DateTime now = DateTime::Now();
    reportFolderPath = FilePath(Format("~doc:/PerformanceReports/Report_%u/", now.GetTimestamp()));
    FileSystem::Instance()->CreateDirectory(reportFolderPath, true);
    reportFile = File::Create(reportFolderPath + "report.txt", File::CREATE | File::WRITE);

    screenshotsToStart.clear();
    screenshotsToSave.clear();

    sampleIndex = 0;
    SetSamplePosition(samples[sampleIndex]);

    return true;
}

void GridTest::Stop()
{
    if (state == Running)
    {
        DAVA::FileSystem::Instance()->DeleteDirectory(reportFolderPath);
        state = Finished;
    }
    else if (state == MakingScreenshots)
    {
        screenshotsToStart.clear();
        // as soon as currently processing screenshots will be saved, class state changes to Finished
    }
}

void GridTest::SetSamplePosition(Sample& sample)
{
    DAVA::Camera* camera = scene->GetCurrentCamera();
    camera->SetPosition(DAVA::Vector3(sample.pos.x, sample.pos.y, sample.pos.z));
    camera->SetDirection(DAVA::Vector3(sample.cos, sample.sine, 0));
}

void GridTest::MakeScreenshot(ScreenshotSaver& screenshotSaver)
{
    SetSamplePosition(screenshotSaver.sample);
    DAVA::UIControlSystem::Instance()->GetScreenshoter()->MakeScreenshot(sceneView, DAVA::FORMAT_RGBA8888, MakeFunction(&screenshotSaver, &ScreenshotSaver::SaveTexture));
}

void GridTest::RemoveSavedScreenshots()
{
    auto it = screenshotsToSave.begin();
    while (it != screenshotsToSave.end())
    {
        ScreenshotSaver& screenshotSaver = *it;
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

void GridTest::ProcessMeasuredSamples()
{
    using namespace DAVA;

    float32 avgFps = 0.0;
    float32 minFps = 60.0;
    float32 maxFps = 0.0;
    for (uint32 sampleIndex = 0; sampleIndex < samples.size(); ++sampleIndex)
    {
        Sample& sample = samples[sampleIndex];

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
            screenshotsToStart.emplace_back(ScreenshotSaver(path, sample));
        }
    }
    avgFps /= samples.size();
    String total = Format("Avg fps: %.1f, min %.1f, max %.1f", avgFps, minFps, maxFps);
    Logger::Info("%s", total.c_str());
    reportFile->WriteLine(total);
    reportFile.reset();

    SetState(State::MakingScreenshots);
}

void GridTest::Update(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    switch (state)
    {
    case State::Running:
    {
        fpsMeter.Update(timeElapsed);
        if (fpsMeter.IsFpsReady())
        {
            Sample& sample = samples[sampleIndex];
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
    case State::MakingScreenshots:
    {
        isEvenFrame = !isEvenFrame;
        if (!screenshotsToStart.empty())
        {
            if (isEvenFrame) // making screenshots only on every other frame (here on each even frame). It's a hack to avoid bugs in screenshot maker
            {
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
                SetState(State::Finished);
            }
        }

        return;
    }
    case State::Finished:
    default:
    {
        return;
    }
    }
}

void GridTest::SetState(State newState)
{
    State prevState = state;
    state = newState;

    if (listener && newState != prevState)
    {
        listener->OnGridTestStateChanged();
    }
}
