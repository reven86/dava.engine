#include "OverdrawTesterSystem.h"

#include <random>
#include <numeric>

#include "OverdrawTesterComponent.h"
#include "OverdrawTesterRenderObject.h"

#include "Base/String.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Texture.h"
#include "Time/SystemTimer.h"
#include "Utils/StringFormat.h"

namespace OverdrawPerformanceTester
{
using DAVA::Array;
using DAVA::String;

using DAVA::FastName;
using DAVA::Texture;
using DAVA::NMaterial;

using DAVA::uint32;
using DAVA::int32;
using DAVA::float32;
using DAVA::uint8;
using DAVA::Vector4;

using DAVA::Scene;
using DAVA::Function;
using DAVA::Entity;
using DAVA::PixelFormat;

const Array<FastName, 4> OverdrawTesterSystem::textureNames =
{ {
    FastName("t1"),
    FastName("t2"),
    FastName("t3"),
    FastName("t4")
} };

const uint8 OverdrawTesterSystem::maxTexturesCount = 4;
const FastName OverdrawTesterSystem::materialPath("~res:/CustomMaterials/OverdrawTester.material");
const FastName OverdrawTesterSystem::sampleCountKeyword("SAMPLE_COUNT");
const FastName OverdrawTesterSystem::dependentReadKeyword("DEPENDENT_READ_TEST");
const uint32 OverdrawTesterSystem::accumulatedFramesCount = 20;
const PixelFormat OverdrawTesterSystem::texureFormat = DAVA::FORMAT_RGBA8888;

OverdrawTesterSystem::OverdrawTesterSystem(DAVA::Scene* scene, DAVA::Function<void(DAVA::Array<DAVA::Vector<FrameData>, 6>*)> finishCallback_)
    : SceneSystem(scene), finishCallback(finishCallback_)
{
    overdrawMaterial = new NMaterial();
    overdrawMaterial->SetFXName(materialPath);
    overdrawMaterial->AddFlag(FastName("SAMPLE_COUNT"), 0);
    overdrawMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);
    
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist255(1, 255);

    for (uint32 i = 0; i < maxTexturesCount; i++)
    {
        textures.push_back(GenerateTexture(rng, dist255));
    }
}

OverdrawTesterSystem::~OverdrawTesterSystem()
{
    SafeRelease(overdrawMaterial);
    for (auto tex : textures)
    {
        SafeRelease(tex);
    }
    textures.clear();
}

void OverdrawTesterSystem::AddEntity(DAVA::Entity* entity)
{
    OverdrawTesterComonent* comp = static_cast<OverdrawTesterComonent*>(entity->GetComponent(OverdrawTesterComonent::OVERDRAW_TESTER_COMPONENT));
    if (comp != nullptr)
    {
        maxStepsCount = comp->GetStepsCount();
        overdrawPercent = comp->GetStepOverdraw();

        OverdrawTesterRenderObject* renderObject = comp->GetRenderObject();
        renderObject->SetDrawMaterial(overdrawMaterial);
        GetScene()->GetRenderSystem()->RenderPermanent(renderObject);
        activeRenderObjects.push_back(renderObject);
    }
}

void OverdrawTesterSystem::RemoveEntity(Entity* entity)
{
    OverdrawTesterComonent* comp = static_cast<OverdrawTesterComonent*>(entity->GetComponent(OverdrawTesterComonent::OVERDRAW_TESTER_COMPONENT));
    if (comp != nullptr)
    {
        GetScene()->GetRenderSystem()->RemoveFromRender(comp->GetRenderObject());
        auto it = std::find(activeRenderObjects.begin(), activeRenderObjects.end(), comp->GetRenderObject());

        DVASSERT(it != activeRenderObjects.end());
        activeRenderObjects.erase(it);
    }
}

void OverdrawTesterSystem::Process(DAVA::float32 timeElapsed)
{
    if (isFinished) return;

    static int32 framesDrawn = 0;
    frames[framesDrawn] = DAVA::SystemTimer::GetRealFrameDelta();
    framesDrawn++;
    
//     time += timeElapsed;
    if (framesDrawn == accumulatedFramesCount)
    {
        float32 smoothFrametime = std::accumulate(frames.begin(), frames.end(), 0.0f) / frames.size();
        performanceData[textureSampleCount].push_back({ smoothFrametime, GetCurrentOverdraw() });
        currentStepsCount++;
        framesDrawn = 0;
    }

    if (currentStepsCount > maxStepsCount)
    {
        currentStepsCount = 0;
        textureSampleCount++;
        if (textureSampleCount < maxTexturesCount + 1)
            SetupMaterial(&textureNames[textureSampleCount - 1]);
        else if (textureSampleCount == maxTexturesCount + 1)
            overdrawMaterial->AddFlag(dependentReadKeyword, 1);
        else
        {
            isFinished = true;
            if (finishCallback)
                finishCallback(&performanceData);
        }
    }

    for (auto renderObject : activeRenderObjects)
        renderObject->SetCurrentStepsCount(currentStepsCount);
}

DAVA::Texture* OverdrawTesterSystem::GenerateTexture(std::mt19937& rng, std::uniform_int_distribution<std::mt19937::result_type>& dist255)
{
    static const uint32 width = 2048;
    static const uint32 height = 2048;

    unsigned char* data = new unsigned char[width * height * 4];

    uint32 dataIndex = 0;
    for (uint32 i = 0; i < height; i++)
        for (uint32 j = 0; j < width; j++)
        {
            data[dataIndex++] = static_cast<uint8>(dist255(rng));
            data[dataIndex++] = static_cast<uint8>(dist255(rng));
            data[dataIndex++] = static_cast<uint8>(dist255(rng));
            data[dataIndex++] = static_cast<uint8>(dist255(rng));
        }
    Texture* result = DAVA::Texture::CreateFromData(texureFormat, data, width, height, false);

    delete[] data;
    return result;
}

void OverdrawTesterSystem::SetupMaterial(const DAVA::FastName* texture)
{
    overdrawMaterial->SetFlag(sampleCountKeyword, textureSampleCount);
    overdrawMaterial->AddTexture(*texture, textures[textureSampleCount - 1]);
}
}
