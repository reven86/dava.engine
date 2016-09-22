#include "SpeedTreeUpdateSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Systems/WindSystem.h"
#include "Scene3D/Systems/WaveSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Utils/Random.h"
#include "Math/Math2D.h"
#include "Debug/CPUProfiler.h"
#include "Render/Renderer.h"

namespace DAVA
{
SpeedTreeUpdateSystem::SpeedTreeUpdateSystem(Scene* scene)
    : SceneSystem(scene)
{
    RenderOptions* options = Renderer::GetOptions();
    options->AddObserver(this);
    isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);

    isVegetationAnimationEnabled = QualitySettingsSystem::Instance()->IsOptionEnabled(QualitySettingsSystem::QUALITY_OPTION_VEGETATION_ANIMATION);

    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SPEED_TREE_MAX_ANIMATED_LOD_CHANGED);
}

SpeedTreeUpdateSystem::~SpeedTreeUpdateSystem()
{
    DVASSERT(allTrees.size() == 0);

    Renderer::GetOptions()->RemoveObserver(this);
}

void SpeedTreeUpdateSystem::ImmediateEvent(Component* _component, uint32 event)
{
    Entity* entity = _component->GetEntity();
    if (event == EventSystem::WORLD_TRANSFORM_CHANGED)
    {
        SpeedTreeComponent* component = GetSpeedTreeComponent(entity);
        if (component)
        {
            Matrix4* wtMxPrt = GetTransformComponent(component->GetEntity())->GetWorldTransformPtr();
            component->wtPosition = wtMxPrt->GetTranslationVector();
            wtMxPrt->GetInverse(component->wtInvMx);
        }
    }
    if (event == EventSystem::SPEED_TREE_MAX_ANIMATED_LOD_CHANGED)
    {
        UpdateAnimationFlag(_component->GetEntity());
    }
}

void SpeedTreeUpdateSystem::AddEntity(Entity* entity)
{
    SpeedTreeComponent* component = GetSpeedTreeComponent(entity);
    DVASSERT(component);
    component->leafTime = static_cast<float32>(Random::Instance()->RandFloat(1000.f));
    allTrees.push_back(component);
}

void SpeedTreeUpdateSystem::RemoveEntity(Entity* entity)
{
    uint32 treeCount = static_cast<uint32>(allTrees.size());
    for (uint32 i = 0; i < treeCount; ++i)
    {
        if (allTrees[i]->entity == entity)
        {
            RemoveExchangingWithLast(allTrees, i);
            break;
        }
    }
}

void SpeedTreeUpdateSystem::UpdateAnimationFlag(Entity* entity)
{
    DVASSERT(GetRenderObject(entity)->GetType() == RenderObject::TYPE_SPEED_TREE);
    SpeedTreeObject* treeObject = static_cast<SpeedTreeObject*>(GetRenderObject(entity));
    SpeedTreeComponent* component = GetSpeedTreeComponent(entity);

    int32 lodIndex = (isAnimationEnabled && isVegetationAnimationEnabled) ? component->GetMaxAnimatedLOD() : -1;
    treeObject->UpdateAnimationFlag(lodIndex);
}

void SpeedTreeUpdateSystem::Process(float32 timeElapsed)
{
    DAVA_CPU_PROFILER_SCOPE("SpeedTreeUpdateSystem::Process");

    if (!isAnimationEnabled || !isVegetationAnimationEnabled)
        return;

    WindSystem* windSystem = GetScene()->windSystem;
    WaveSystem* waveSystem = GetScene()->waveSystem;

    //Update trees
    uint32 treeCount = static_cast<uint32>(allTrees.size());
    for (uint32 i = 0; i < treeCount; ++i)
    {
        SpeedTreeComponent* component = allTrees[i];
        DVASSERT(GetRenderObject(component->GetEntity())->GetType() == RenderObject::TYPE_SPEED_TREE);
        SpeedTreeObject* treeObject = static_cast<SpeedTreeObject*>(GetRenderObject(component->GetEntity()));

        if (component->GetMaxAnimatedLOD() < treeObject->GetLodIndex())
            continue;

        const Vector3& treePosition = component->wtPosition;
        Vector3 wind3D = windSystem->GetWind(treePosition) + waveSystem->GetWaveDisturbance(treePosition);
        float32 leafForce = wind3D.Length();
        Vector2 windVec(wind3D.x, wind3D.y);

        component->oscVelocity += (windVec - component->oscOffset * component->GetTrunkOscillationSpring()) * timeElapsed;

        float32 velocityLengthSq = component->oscVelocity.SquareLength();
        Vector2 dVelocity = (component->GetTrunkOscillationDamping() * std::sqrt(velocityLengthSq) * component->oscVelocity) * timeElapsed;
        if (velocityLengthSq >= dVelocity.SquareLength())
            component->oscVelocity -= dVelocity;
        else
            component->oscVelocity = Vector2();

        component->oscOffset += component->oscVelocity * timeElapsed;

        component->leafTime += timeElapsed * std::sqrt(leafForce) * component->GetLeafsOscillationSpeed();

        float32 sine, cosine;
        SinCosFast(component->leafTime, sine, cosine);
        float32 leafsOscillationAmplitude = component->GetLeafsOscillationApmlitude();
        Vector2 leafOscillationParams(leafsOscillationAmplitude * sine, leafsOscillationAmplitude * cosine);

        Vector2 localOffset = MultiplyVectorMat2x2(component->oscOffset * component->GetTrunkOscillationAmplitude(), component->wtInvMx);
        treeObject->SetTreeAnimationParams(localOffset, leafOscillationParams);
    }
}

void SpeedTreeUpdateSystem::HandleEvent(Observable* observable)
{
    RenderOptions* options = static_cast<RenderOptions*>(observable);

    if (isAnimationEnabled != options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS))
    {
        isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);

        uint32 treeCount = static_cast<uint32>(allTrees.size());
        for (uint32 i = 0; i < treeCount; ++i)
        {
            UpdateAnimationFlag(allTrees[i]->entity);
        }
    }
}

void SpeedTreeUpdateSystem::SceneDidLoaded()
{
    for (auto tree : allTrees)
    {
        auto renderComponent = GetRenderComponent(tree->entity);
        if (renderComponent != nullptr)
        {
            auto ro = renderComponent->GetRenderObject();
            if (ro != nullptr)
            {
                ro->RecalcBoundingBox();
            }
        }
    }
}
};