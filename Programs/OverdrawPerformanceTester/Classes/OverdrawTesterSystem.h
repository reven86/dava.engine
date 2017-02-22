#pragma once

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
    OverdrawTesterSystem(DAVA::Scene* scene, DAVA::Function<void()> finishCallback_);
    ~OverdrawTesterSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    DAVA::WideString GetInfoString();

    virtual void Process(DAVA::float32 timeElapsed) override;

    inline DAVA::float32 GetCurrentOverdraw() const;
    inline DAVA::uint32 GetCurrentSampleCount() const;

private:
    DAVA::Texture* GenerateTexture(DAVA::Vector4 startColor, DAVA::Vector4 endColor);
    void SetupMaterial(const DAVA::FastName* keyword, const DAVA::FastName* texture);

    DAVA::Vector<OverdrawTesterRenderObject*> activeRenderObjects;
    DAVA::NMaterial* overdrawMaterial;
    DAVA::uint32 currentStepsCount = 1;
    DAVA::uint32 maxStepsCount = 100;
    DAVA::uint32 textureSampleCount = 0;
    DAVA::float32 overdrawPercent = 10.0f;

    DAVA::Vector<DAVA::Texture*> textures;

    bool finished = false;

    DAVA::Function<void()> finishCallback;

    static const DAVA::Array<DAVA::FastName, 4> keywords;
    static const DAVA::Array<DAVA::FastName, 4> textureNames;
};

DAVA::float32 OverdrawTesterSystem::GetCurrentOverdraw() const
{
    return overdrawPercent * currentStepsCount;
}

DAVA::uint32 OverdrawTesterSystem::GetCurrentSampleCount() const
{
    return textureSampleCount;
}
}