#include "Scene3D/Lod/LodComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
const float32 LodComponent::INVALID_DISTANCE = -1.f;
const float32 LodComponent::MIN_LOD_DISTANCE = 0.f;
const float32 LodComponent::MAX_LOD_DISTANCE = 1000.f;

const float32 NEAR_DISTANCE_COEFF = 1.05f;
const float32 FAR_DISTANCE_COEFF = 0.95f;

Component* LodComponent::Clone(Entity* toEntity)
{
    LodComponent* newLod = new LodComponent();
    newLod->SetEntity(toEntity);

    newLod->distances = distances;

    return newLod;
}

void LodComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (NULL != archive)
    {
        uint32 i;

        KeyedArchive* lodDistArch = new KeyedArchive();
        for (i = 0; i < MAX_LOD_LAYERS; ++i)
        {
            KeyedArchive* lodDistValuesArch = new KeyedArchive();
            lodDistValuesArch->SetFloat("ld.distance", distances[i]);

            lodDistArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), lodDistValuesArch);
            lodDistValuesArch->Release();
        }
        archive->SetArchive("lc.loddist", lodDistArch);
        lodDistArch->Release();
    }
}

void LodComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        KeyedArchive* lodDistArch = archive->GetArchive("lc.loddist");
        if (NULL != lodDistArch)
        {
            if (serializationContext->GetVersion() < 19) //before lodsystem refactoring
            {
                for (uint32 i = 1; i < MAX_LOD_LAYERS; ++i)
                {
                    KeyedArchive* lodDistValuesArch = lodDistArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
                    if (NULL != lodDistValuesArch)
                    {
                        distances[i - 1] = lodDistValuesArch->GetFloat("ld.distance");
                    }
                }
                distances[MAX_LOD_LAYERS - 1] = std::numeric_limits<float32>::max();
                for (uint32 i = 1; i < MAX_LOD_LAYERS; ++i)
                {
                    if (distances[i] < distances[i - 1])
                    {
                        distances[i] = std::numeric_limits<float32>::max();
                    }
                }
            }
            else
            {
                for (uint32 i = 0; i < MAX_LOD_LAYERS; ++i)
                {
                    KeyedArchive* lodDistValuesArch = lodDistArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
                    if (NULL != lodDistValuesArch)
                    {
                        distances[i] = lodDistValuesArch->GetFloat("ld.distance");
                    }
                }
            }
        }
    }

    Component::Deserialize(archive, serializationContext);
}

LodComponent::LodComponent()
    : currentLod(INVALID_LOD_LAYER)
{
    distances.resize(LodComponent::MAX_LOD_LAYERS);
    for (int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
    {
        SetLodLayerDistance(iLayer, GetDefaultDistance(iLayer));
    }
}

float32 LodComponent::GetDefaultDistance(int32 layer)
{
    float32 distance = MIN_LOD_DISTANCE + ((MAX_LOD_DISTANCE - MIN_LOD_DISTANCE) / (MAX_LOD_LAYERS - 1)) * (layer + 1);
    return distance;
}

void LodComponent::SetLodLayerDistance(int32 layerNum, float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    distances[layerNum] = distance;
    GlobalEventSystem::Instance()->Event(this, EventSystem::LOD_DISTANCE_CHANGED);
}

void LodComponent::EnableRecursiveUpdate()
{
    GlobalEventSystem::Instance()->Event(this, EventSystem::LOD_RECURSIVE_UPDATE_ENABLED);
}
};
