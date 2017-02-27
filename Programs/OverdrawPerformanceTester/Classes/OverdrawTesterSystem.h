#pragma once

#include <random>

#include "FrameData.h"

#include "Entity/SceneSystem.h"
#include "Math/Vector.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/String.h"
#include "Functional/Function.h"

namespace DAVA
{
class Scene;
class NMaterial;
class Texture;
}

namespace OverdrawPerformanceTester
{
class OverdrawTesterRenderObject;

class OverdrawTesterSystem : public DAVA::SceneSystem
{
public:
    OverdrawTesterSystem(DAVA::Scene* scene, DAVA::Function<void(DAVA::Array<DAVA::Vector<FrameData>, 6>*)> finishCallback_);
    ~OverdrawTesterSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    virtual void Process(DAVA::float32 timeElapsed) override;

    inline DAVA::float32 GetCurrentOverdraw() const;
    inline DAVA::uint32 GetCurrentSampleCount() const;
    inline void SetIncreasePercentTime(DAVA::float32 time);

private:
    DAVA::Texture* GenerateTexture(std::mt19937& rng, std::uniform_int_distribution<std::mt19937::result_type>& dist255);
    void SetupMaterial(const DAVA::FastName* texture);

    DAVA::Vector<OverdrawTesterRenderObject*> activeRenderObjects;
    DAVA::Array<DAVA::Vector<FrameData>, 6> performanceData;
    DAVA::Vector<DAVA::Texture*> textures;

    DAVA::Function<void(DAVA::Array<DAVA::Vector<FrameData>, 6>*)> finishCallback;

    DAVA::uint32 currentStepsCount = 1;
    DAVA::uint32 maxStepsCount = 100;
    DAVA::uint32 textureSampleCount = 0;
    DAVA::float32 overdrawPercent = 10.0f;
    DAVA::float32 increasePercentTime = 0.05f;

    DAVA::NMaterial* overdrawMaterial;
    DAVA::Array<DAVA::float32, 20> frames;

    bool isFinished = false;

    static const DAVA::Array<DAVA::FastName, 4> textureNames;
    static const DAVA::uint8 maxTexturesCount;
    static const DAVA::FastName materialPath;
    static const DAVA::FastName sampleCountKeyword;
    static const DAVA::FastName dependentReadKeyword;
    static const DAVA::uint32 accumulatedFramesCount;
    static const DAVA::PixelFormat textureFormat;
    static const bool generateTexWithMips;
};

DAVA::float32 OverdrawTesterSystem::GetCurrentOverdraw() const
{
    return overdrawPercent * currentStepsCount;
}

DAVA::uint32 OverdrawTesterSystem::GetCurrentSampleCount() const
{
    return textureSampleCount;
}

void OverdrawTesterSystem::SetIncreasePercentTime(DAVA::float32 time)
{
    increasePercentTime = time;
}
}