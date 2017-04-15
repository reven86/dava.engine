#include "GridTest.h"

#include <Base/BaseTypes.h>
#include <Render/Texture.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageConvert.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/Camera.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Time/DateTime.h>
#include <UI/UIControlSystem.h>
#include <UI/Render/UIRenderSystem.h>
#include <UI/UIScreenshoter.h>
#include <Logger/Logger.h>
#include <Math/Vector.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/File.h>
#include <Utils/StringFormat.h>
#include <Utils/FpsMeter.h>

namespace GridTestDetails
{
using namespace DAVA;

const float32 LOW_FPS_THRESHOLD = 40.f;

const uint32 GRID_SIZE = 8;
const uint32 ANGLE_COUNT = 8;
const float32 EXPOSURE_DURATION_SEC = 1.f;
const float32 ELEVATION_ABOVE_LANDSCAPE = 10.f;

const uint32 PANORAMA_IMAGE_SIZE = 512;

const float32 ANGLE_STEP_DEGREES = 360.f / ANGLE_COUNT;

void SetSamplePosition(Scene* scene, const GridTestSample& sample)
{
    DVASSERT(scene != nullptr);
    Camera* camera = scene->GetCurrentCamera();
    camera->SetPosition(sample.pos);
    camera->SetDirection(Vector3(sample.cos, sample.sine, 0));
}

class Screenshot
{
public:
    explicit Screenshot(UI3DView* sceneView)
        : sceneView(sceneView)
    {
    }
    virtual ~Screenshot()
    {
    }
    virtual void MakeScreenshot() = 0;
    virtual void SaveScreenshot(Texture* screenshot) = 0;

    UI3DView* sceneView = nullptr;
    bool saved = false;
};

class SectorScreenshot : public Screenshot
{
public:
    explicit SectorScreenshot(UI3DView* sceneView, const GridTestSample& sample);

    void MakeScreenshot() override;
    void SaveScreenshot(Texture* screenshot) override;

    const GridTestSample& sample;
    const FilePath& savePath;
};

SectorScreenshot::SectorScreenshot(UI3DView* sceneView, const GridTestSample& sample)
    : Screenshot(sceneView)
    , sample(sample)
    , savePath(sample.screenshotPath)
{
}

void SectorScreenshot::MakeScreenshot()
{
    SetSamplePosition(sceneView->GetScene(), sample);
    UIScreenshoter* screenshoter = UIControlSystem::Instance()->GetRenderSystem()->GetScreenshoter();
    screenshoter->MakeScreenshot(sceneView, FORMAT_RGBA8888, MakeFunction(this, &GridTestDetails::SectorScreenshot::SaveScreenshot));
}

void SectorScreenshot::SaveScreenshot(Texture* screenshot)
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
    explicit PanoramaScreenshot(UI3DView* sceneView, ScopedPtr<Image>& image, FilePath savePath);
    void MakeScreenshot() override;
    void SaveScreenshot(Texture* screenshot) override;

private:
    ScopedPtr<Image>& image;
    FilePath savePath;
    bool vertexFogWasOverriden = false;
    int32 vertexFogValue = 0;
};

PanoramaScreenshot::PanoramaScreenshot(UI3DView* sceneView, ScopedPtr<Image>& image, FilePath savePath)
    : Screenshot(sceneView)
    , image(image)
{
}

void PanoramaScreenshot::MakeScreenshot()
{
    Scene* scene = sceneView->GetScene();

    Landscape* landscape = FindLandscape(scene);
    float32 landscapeSize = landscape->GetLandscapeSize();

    if (scene->GetGlobalMaterial())
    {
        vertexFogWasOverriden = true;
        vertexFogValue = scene->GetGlobalMaterial()->GetLocalFlagValue(NMaterialFlagName::FLAG_VERTEXFOG);
        scene->GetGlobalMaterial()->SetFlag(NMaterialFlagName::FLAG_VERTEXFOG, 0);
    }
    Camera* camera = scene->GetCurrentCamera();
    camera->SetupOrtho(landscapeSize, 1.f, 1.f, 5000.f);
    camera->SetLeft(Vector3(1.f, 0.f, 0.f));
    camera->SetDirection(Vector3(0.f, 0.f, -1.f));
    camera->SetPosition(Vector3(0.f, 0.f, 60.f));
    camera->SetTarget(Vector3(0.f, 0.1f, 0.f));

    UIScreenshoter* screenshoter = UIControlSystem::Instance()->GetRenderSystem()->GetScreenshoter();
    screenshoter->MakeScreenshot(sceneView, FORMAT_RGBA8888, MakeFunction(this, &PanoramaScreenshot::SaveScreenshot));
}

void PanoramaScreenshot::SaveScreenshot(Texture* screenshot)
{
    if (vertexFogWasOverriden)
    {
        sceneView->GetScene()->GetGlobalMaterial()->SetFlag(NMaterialFlagName::FLAG_VERTEXFOG, vertexFogValue);
    }

    ScopedPtr<Image> screenshotImage(screenshot->CreateImageFromMemory());
    const Size2i& size = UIControlSystem::Instance()->vcs->GetPhysicalScreenSize();
    screenshotImage->ResizeCanvas(static_cast<uint32>(size.dx), static_cast<uint32>(size.dy));

    image.reset(Image::Create(PANORAMA_IMAGE_SIZE, PANORAMA_IMAGE_SIZE, FORMAT_RGBA8888));
    ImageConvert::ResizeRGBA8Billinear(reinterpret_cast<uint32*>(screenshotImage->data), screenshotImage->GetWidth(), screenshotImage->GetHeight(),
                                       reinterpret_cast<uint32*>(image->data), image->width, image->height);

    if (savePath.Exists())
    {
        image->Save(savePath);
    }

    saved = true;
}
}

class GridTestImpl final
{
public:
    explicit GridTestImpl(DAVA::Engine& engine, GridTestListener* listener, GridTest::Mode mode);
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

    GridTestListener* listener = nullptr;
    DAVA::Scene* scene = nullptr;
    DAVA::ScopedPtr<DAVA::UI3DView> sceneView;

    GridTest::Mode mode = GridTest::Mode::ModeDefault;
    GridTest::State state = GridTest::StateFinished;

    DAVA::FpsMeter fpsMeter;

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

GridTestImpl::GridTestImpl(DAVA::Engine& engine, GridTestListener* listener, GridTest::Mode mode)
    : engine(engine)
    , listener(listener)
    , mode(mode)
    , fpsMeter(GridTestDetails::EXPOSURE_DURATION_SEC)
{
    engine.update.Connect(this, &GridTestImpl::Update);
}

GridTestImpl::~GridTestImpl()
{
    engine.update.Disconnect(this);
    Stop();
}

bool GridTestImpl::Start(const DAVA::ScopedPtr<DAVA::UI3DView>& view)
{
    using namespace GridTestDetails;
    using namespace DAVA;

    if (state != GridTest::StateFinished)
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

                float32 landscapeHeight = 0.f;
                landscape->GetHeightAtPoint(testPosition.pos, landscapeHeight);
                testPosition.pos.z = landscapeHeight + ELEVATION_ABOVE_LANDSCAPE;

                testPosition.angle = angle;
                SinCosFast(DegToRad(angle), testPosition.sine, testPosition.cos);
            }
        }

        invertedDirection = !invertedDirection;
    }

    SetState(GridTest::StateRunning);

    if (mode == GridTest::ModeGenerateReport)
    {
        DateTime now = DateTime::Now();
        reportFolderPath = FilePath(Format("~doc:/PerformanceReports/GridTest_%u/", now.GetTimestamp()));
        FileSystem::Instance()->CreateDirectory(reportFolderPath, true);
        reportFile = File::Create(reportFolderPath + "report.txt", File::CREATE | File::WRITE);
    }

    framesSinceLastSave = 0;
    screenshotsToStart.clear();
    screenshotsToSave.clear();

    currentSampleIndex = 0;
    GridTestDetails::SetSamplePosition(scene, result.samples[currentSampleIndex]);

    return true;
}

void GridTestImpl::Stop()
{
    if (state == GridTest::StateRunning)
    {
        if (mode == GridTest::ModeGenerateReport)
        {
            DAVA::FileSystem::Instance()->DeleteDirectory(reportFolderPath);
        }
        state = GridTest::StateFinished;
    }
    else if (state == GridTest::StateMakingScreenshots)
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

        result.avgFPS += sample.fps;

        if (sample.fps < result.minFPS)
            result.minFPS = sample.fps;

        if (sample.fps > result.maxFPS)
            result.maxFPS = sample.fps;

        if (mode == GridTest::ModeGenerateReport)
        {
            reportFile->WriteLine(Format("Sample %.0f.%.0f.%.0f angle %.0f-%.0f: fps %.1f", sample.pos.x, sample.pos.y, sample.pos.z, sample.cos, sample.sine, sample.fps));

            if (sample.fps < GridTestDetails::LOW_FPS_THRESHOLD)
            {
                String screenshotName = Format("screenshot_fps%2.0f_frame%u.png", sample.fps, sampleIndex);
                sample.screenshotPath = reportFolderPath + screenshotName;
                screenshotsToStart.emplace_back(new GridTestDetails::SectorScreenshot(sceneView, sample));
            }
        }
    }

    result.avgFPS /= result.samples.size();

    if (mode == GridTest::ModeGenerateReport)
    {
        String total = Format("Avg fps: %.1f, min %.1f, max %.1f", result.avgFPS, result.minFPS, result.maxFPS);
        reportFile->WriteLine(total);
        reportFile.reset();

        result.panoramaPath = reportFolderPath + "panorama.png";
    }

    screenshotsToStart.emplace_back(new GridTestDetails::PanoramaScreenshot(sceneView, result.panoramaImage, result.panoramaPath));

    SetState(GridTest::StateMakingScreenshots);
}

void GridTestImpl::Update(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    switch (state)
    {
    case GridTest::StateRunning:
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
    case GridTest::StateMakingScreenshots:
    {
        if (!screenshotsToStart.empty())
        {
            if (framesSinceLastSave++ == 2) // making screenshots only on every 2th frame, to avoid out of memory errors.
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
                SetState(GridTest::StateFinished);
            }
        }

        return;
    }
    case GridTest::StateFinished:
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

GridTest::GridTest(DAVA::Engine& engine, GridTestListener* listener, Mode mode)
    : impl(new GridTestImpl(engine, listener, mode))
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
