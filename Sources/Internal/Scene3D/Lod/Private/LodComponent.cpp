#include "Scene3D/Lod/LodComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
const float32 LodComponent::INVALID_DISTANCE = -1.f;
const float32 LodComponent::MIN_LOD_DISTANCE = 0.f;
const float32 LodComponent::MAX_LOD_DISTANCE = 1000.f;

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
};
