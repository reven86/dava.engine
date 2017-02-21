#pragma once

#include "Entity/SceneSystem.h"
#include "Math/Vector.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"

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
    OverdrawTesterSystem(DAVA::Scene* scene);
    ~OverdrawTesterSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    virtual void Process(DAVA::float32 timeElapsed) override;

private:
    DAVA::Texture* GenerateTexture(DAVA::Vector4 startColor, DAVA::Vector4 endColor);

    DAVA::Vector<OverdrawTesterRenderObject*> activeRenderObjects;
    DAVA::NMaterial* overdrawMaterial;
    DAVA::uint32 stepsCount = 1;
    DAVA::uint32 maxStepsCount = 100;
    DAVA::uint32 textureSamples = 0;

    DAVA::Vector<DAVA::Texture*> textures;
    DAVA::Texture* t1;
    DAVA::Texture* t2;
    DAVA::Texture* t3;
    DAVA::Texture* t4;

    bool finished = false;
    void SetupMaterial(const DAVA::FastName* keyword, const DAVA::FastName* texture);

    static const DAVA::Array<DAVA::FastName, 4> keywords;
    static const DAVA::Array<DAVA::FastName, 4> textureNames;
};
}