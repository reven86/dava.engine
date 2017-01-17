#include "GridTest.h"

#include <Base/BaseTypes.h>
#include <Render/Texture.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageConvert.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/Camera.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Platform/DateTime.h>
#include <UI/UIControlSystem.h>
#include <UI/UIScreenshoter.h>
#include <Logger/Logger.h>
#include <Math/Vector.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/File.h>
#include <Functional/SignalBase.h>
#include <Utils/StringFormat.h>
#include <Utils/FpsMeter.h>

namespace GridTestDetails
{
using namespace DAVA;

const float32 LOW_FPS_THRESHOLD = 57.0f;

const uint32 GRID_SIZE = 8;
const uint32 ANGLE_COUNT = 8;
const float32 EXPOSURE_DURATION_SEC = 0.1f;
const float32 ELEVATION_ABOVE_LANDSCAPE = 10.f;

const uint32 PANORAMA_IMAGE_SIZE = 512;

const float32 ANGLE_STEP_DEGREES = 360.f / ANGLE_COUNT;

void SetSamplePosition(Scene* scene, GridTestSample& sample)
{
    DVASSERT(scene != nullptr);
    Camera* camera = scene->GetCurrentCamera();
    camera->SetPosition(Vector3(sample.pos.x, sample.pos.y, sample.pos.z));
    camera->SetDirection(Vector3(sample.cos, sample.sine, 0));
}

class Screenshot
{
public:
    explicit Screenshot(UI3DView* sceneView, FilePath& path);
    virtual void MakeScreenshot() = 0;
    virtual void SaveTexture(Texture* screenshot) = 0;

    UI3DView* sceneView = nullptr;
    FilePath savePath;
    bool saved = false;
};

Screenshot::Screenshot(UI3DView* sceneView, FilePath& path)
    : sceneView(sceneView)
    , savePath(path)
{
}

class SectorScreenshot : public Screenshot
{
public:
    explicit SectorScreenshot(UI3DView* sceneView, GridTestSample& sample);

    void MakeScreenshot() override;
    void SaveTexture(Texture* screenshot) override;

    GridTestSample sample;
};

SectorScreenshot::SectorScreenshot(UI3DView* sceneView, GridTestSample& sample)
    : Screenshot(sceneView, sample.screenshotPath)
    , sample(sample)
{
}

void SectorScreenshot::MakeScreenshot()
{
    SetSamplePosition(sceneView->GetScene(), sample);
    UIControlSystem::Instance()->GetScreenshoter()->MakeScreenshot(sceneView, FORMAT_RGBA8888, MakeFunction(this, &GridTestDetails::SectorScreenshot::SaveTexture));
}

void SectorScreenshot::SaveTexture(Texture* screenshot)
{
    ScopedPtr<Image> image(screenshot->CreateImageFromMemory());
    const Size2i& size = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();
    image->ResizeCanvas(static_cast<uint32>(size.dx), static_cast<uint32>(size.dy));
    image->Save(savePath);
    saved = true;
}

class PanoramaScreenshot : public Screenshot
{
public:
    explicit PanoramaScreenshot(UI3DView* sceneView, FilePath& path);
    void MakeScreenshot() override;
    void SaveTexture(Texture* screenshot) override;
};

PanoramaScreenshot::PanoramaScreenshot(UI3DView* sceneView, FilePath& path)
    : Screenshot(sceneView, path)
{
}

void PanoramaScreenshot::MakeScreenshot()
{
    Scene* scene = sceneView->GetScene();

    Landscape* landscape = FindLandscape(scene);
    float32 landscapeSize = landscape->GetLandscapeSize();

    if (scene->GetGlobalMaterial())
    {
        scene->GetGlobalMaterial()->SetFlag(NMaterialFlagName::FLAG_VERTEXFOG, 0);
    }
    Camera* camera = scene->GetCurrentCamera();
    camera->SetupOrtho(landscapeSize, 1.f, 1, 5000);
    camera->SetLeft(Vector3(1.f, 0, 0));
    camera->SetDirection(Vector3(0, 0, -1.f));
    camera->SetPosition(Vector3(0, 0, 60.f));
    camera->SetTarget(Vector3(0.f, 0.1f, 0.f));

    UIControlSystem::Instance()->GetScreenshoter()->MakeScreenshot(sceneView, FORMAT_RGBA8888, MakeFunction(this, &PanoramaScreenshot::SaveTexture));
}

#include <Render/Image/ImageConvert.h>

void PanoramaScreenshot::SaveTexture(Texture* screenshot)
{
    ScopedPtr<Image> image(screenshot->CreateImageFromMemory());
    const Size2i& size = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();
    image->ResizeCanvas(static_cast<uint32>(size.dx), static_cast<uint32>(size.dy));
    ScopedPtr<Image> i(Image::Create(PANORAMA_IMAGE_SIZE, PANORAMA_IMAGE_SIZE, FORMAT_RGBA8888));
    ImageConvert::ResizeRGBA8Billinear(reinterpret_cast<uint32*>(image->data), image->GetWidth(), image->GetHeight(),
        reinterpret_cast<uint32*>(i->data), i->width, i->height);
    i->Save(savePath);
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
    GridTestResult& GetResult();

private:
    void Update(DAVA::float32 timeElapsed);

    void ClearResults();
    void MakeScreenshot(GridTestDetails::SectorScreenshot&);
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

    GridTestResult result;
    DAVA::uint32 currentSampleIndex = 0;

    DAVA::List<std::unique_ptr<GridTestDetails::Screenshot>> screenshotsToStart;
    DAVA::List<std::unique_ptr<GridTestDetails::Screenshot>> screenshotsToSave;
    DAVA::uint32 framesSinceLastSave = 0;

    DAVA::FilePath reportFolderPath;
    DAVA::ScopedPtr<DAVA::File> reportFile;
};

GridTest::State GridTestImpl::GetState() const
{
    return state;
}

GridTestResult& GridTestImpl::GetResult()
{
    return result;
}

void GridTestImpl::ClearResults()
{
    result.panoramaPath = "";
    result.samples.clear();
    result.avgFPS = result.minFPS = result.maxFPS = 0.f;
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

    ClearResults();
    result.samples.reserve(GRID_SIZE * GRID_SIZE * ANGLE_COUNT);

    float32 landscapeSize = landscape->GetLandscapeSize();
    float32 step = landscapeSize / (GRID_SIZE + 1);
    float32 sceneMin = -landscapeSize / 2;
    float32 sceneMax = landscapeSize / 2;

    result.sceneSize = landscapeSize;
    result.sceneMin = sceneMin;
    result.sceneMax = sceneMax;
    result.gridStep = step;
    result.sampleAngleDegrees = ANGLE_STEP_DEGREES;

    float32 xMin = sceneMin + step;
    float32 xMax = sceneMax - step;
    float32 yMin = sceneMin + step;

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
                result.samples.push_back(GridTestSample());
                GridTestSample& testPosition = result.samples.back();

                testPosition.pos.x = xPos;
                testPosition.pos.y = yPos;

                float32 landscapeHeight = 0.0;
                landscape->GetHeightAtPoint(testPosition.pos, landscapeHeight);
                testPosition.pos.z = landscapeHeight + ELEVATION_ABOVE_LANDSCAPE;

                testPosition.angle = angle;
                SinCosFast(DegToRad(angle), testPosition.sine, testPosition.cos);
            }
        }

        invertedDirection = !invertedDirection;
    }

    SetState(GridTest::State::Running);
    DateTime now = DateTime::Now();
    reportFolderPath = FilePath(Format("~doc:/PerformanceReports/GridTest_%u/", now.GetTimestamp()));
    FileSystem::Instance()->CreateDirectory(reportFolderPath, true);
    reportFile = File::Create(reportFolderPath + "report.txt", File::CREATE | File::WRITE);

    framesSinceLastSave = 0;
    screenshotsToStart.clear();
    screenshotsToSave.clear();

    currentSampleIndex = 0;
    GridTestDetails::SetSamplePosition(scene, result.samples[currentSampleIndex]);

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

void GridTestImpl::RemoveSavedScreenshots()
{
    auto it = screenshotsToSave.begin();
    while (it != screenshotsToSave.end())
    {
        GridTestDetails::Screenshot& screenshotSaver = **it;
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

    if (result.samples.empty())
    {
        DVASSERT(false);
        return;
    }

    result.minFPS = 100.f;
    result.maxFPS = 0.f;
    result.avgFPS = 0.f;

    for (uint32 sampleIndex = 0; sampleIndex < result.samples.size(); ++sampleIndex)
    {
        GridTestSample& sample = result.samples[sampleIndex];

        reportFile->WriteLine(Format("Sample %.0f.%.0f.%.0f angle %.0f-%.0f: fps %.1f", sample.pos.x, sample.pos.y, sample.pos.z, sample.cos, sample.sine, sample.fps));

        result.avgFPS += sample.fps;

        if (sample.fps < result.minFPS)
            result.minFPS = sample.fps;

        if (sample.fps > result.maxFPS)
            result.maxFPS = sample.fps;

        if (sample.fps < GridTestDetails::LOW_FPS_THRESHOLD)
        {
            String screenshotName = Format("screenshot_fps%2.0f_frame%u.png", sample.fps, sampleIndex);
            sample.screenshotPath = reportFolderPath + screenshotName;
            screenshotsToStart.emplace_back(new GridTestDetails::SectorScreenshot(sceneView, sample));
        }
    }

    result.panoramaPath = reportFolderPath + "panorama.png";
    screenshotsToStart.emplace_back(new GridTestDetails::PanoramaScreenshot(sceneView, result.panoramaPath));

    result.avgFPS /= result.samples.size();
    String total = Format("Avg fps: %.1f, min %.1f, max %.1f", result.avgFPS, result.minFPS, result.maxFPS);
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
            GridTestSample& sample = result.samples[currentSampleIndex];
            sample.fps = fpsMeter.GetFps();

            if (++currentSampleIndex < result.samples.size())
            {
                GridTestDetails::SetSamplePosition(scene, result.samples[currentSampleIndex]);
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
            if (framesSinceLastSave++ == 5) // making screenshots only on every 5th frame. It's a hack to avoid out of memory errors.
            {
                framesSinceLastSave = 0;

                // move first element from screenshotsToStart to the end of screenshotsToSave
                screenshotsToSave.splice(screenshotsToSave.end(), screenshotsToStart, screenshotsToStart.begin());
                screenshotsToSave.back()->MakeScreenshot();
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

GridTestResult& GridTest::GetResult()
{
    return impl->GetResult();
}
