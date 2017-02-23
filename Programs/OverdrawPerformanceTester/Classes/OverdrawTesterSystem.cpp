#include "OverdrawTesterSystem.h"

#include <random>

#include "OverdrawTesterComponent.h"
#include "OverdrawTesterRenderObject.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Texture.h"
#include "OverdrawTesterRenderObject.h"
#include "Utils/StringFormat.h"
#include "Logger/Logger.h"
#include "Time/SystemTimer.h"

namespace OverdrawPerformanceTester
{
using DAVA::Array;

using DAVA::FastName;
using DAVA::Texture;
using DAVA::NMaterial;

using DAVA::uint32;
using DAVA::float32;
using DAVA::uint8;
using DAVA::Vector4;

const Array<FastName, 4> OverdrawTesterSystem::keywords =
{ {
    FastName("ONE_TEX"),
    FastName("TWO_TEX"),
    FastName("THREE_TEX"),
    FastName("FOUR_TEX")
} };

const Array<FastName, 4> OverdrawTesterSystem::textureNames =
{ {
    FastName("t1"),
    FastName("t2"),
    FastName("t3"),
    FastName("t4")
} };

OverdrawTesterSystem::OverdrawTesterSystem(DAVA::Scene* scene, DAVA::Function<void(DAVA::Array<DAVA::Vector<FrameData>, 6>*)> finishCallback_)
    : SceneSystem(scene), finishCallback(finishCallback_)
{
    overdrawMaterial = new NMaterial();
    overdrawMaterial->SetFXName(FastName("~res:/CustomMaterials/OverdrawTester.material"));
    overdrawMaterial->PreBuildMaterial(DAVA::PASS_FORWARD);

    Array<Vector4, 8> colors =
    { {
        Vector4(1.0f * 255, 0.2f * 255, 0.2f, 0.2f * 255), Vector4(0.4f * 255, 0.1f * 255, 1.0f * 255, 0.4f * 255),
        Vector4(0.2f * 255, 1.0f * 255, 0.2f, 0.3f * 255), Vector4(1.0f * 255, 0.4f * 255, 0.4f * 255, 0.1f * 255),
        Vector4(1.0f * 255, 0.2f * 255, 1.0f, 0.1f * 255), Vector4(0.1f * 255, 1.0f * 255, 0.2f * 255, 0.2f * 255),
        Vector4(1.0f * 255, 0.0f * 255, 0.0f, 0.2f * 255), Vector4(0.0f * 255, 0.0f * 255, 1.0f * 255, 0.3f * 255)
    } };


    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist255(1, 255);
    for (uint32 i = 0; i < 8; i += 2)
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

void OverdrawTesterSystem::RemoveEntity(DAVA::Entity* entity)
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

DAVA::WideString OverdrawTesterSystem::GetInfoString()
{
    return textureSampleCount != 5 ? DAVA::Format(L" ||| Texture samples: %d ||| Overdraw %d%", textureSampleCount, static_cast<DAVA::int32>(overdrawPercent*currentStepsCount)) : 
        DAVA::Format(L" ||| Dependent read ||| Overdraw %d%", static_cast<DAVA::int32>(overdrawPercent*currentStepsCount));
}

void OverdrawTesterSystem::Process(DAVA::float32 timeElapsed)
{
    if (finished) return;

    static float32 i = 0;
    i += timeElapsed;
    if (i >= increasePercentTime)
    {
        performanceData[textureSampleCount].push_back({ DAVA::SystemTimer::GetRealFrameDelta(), GetCurrentOverdraw() });
        currentStepsCount++;
        i -= increasePercentTime;
    }
    if (currentStepsCount > maxStepsCount)
    {
        currentStepsCount = 0;
        i = 0;
        textureSampleCount++;
        if (textureSampleCount < 5)
            SetupMaterial(&keywords[textureSampleCount - 1], &textureNames[textureSampleCount - 1]);
        else if (textureSampleCount == 5)
            overdrawMaterial->AddFlag(FastName("DEPENDENT_READ_TEST"), 1);
        else
        {
            finished = true;
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
//             Vector4 finalColor = Lerp(startColor, endColor, static_cast<float>(j) / width);
            data[dataIndex++] = static_cast<uint8>(dist255(rng));
            data[dataIndex++] = static_cast<uint8>(dist255(rng));
            data[dataIndex++] = static_cast<uint8>(dist255(rng));
            data[dataIndex++] = static_cast<uint8>(20);
        }
    Texture* result = DAVA::Texture::CreateFromData(DAVA::FORMAT_RGBA8888, data, width, height, false);
    delete[] data;

    return result;
}

void OverdrawTesterSystem::SetupMaterial(const DAVA::FastName* keyword, const DAVA::FastName* texture)
{
    overdrawMaterial->AddFlag(*keyword, 1);
    overdrawMaterial->AddTexture(*texture, textures[textureSampleCount - 1]);
}

}
