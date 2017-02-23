#include "Commands2/ParticleLayerCommands.h"
#include "Commands2/RECommandIDs.h"

#include "FileSystem/FilePath.h"
#include "Particles/ParticleLayer.h"

using namespace DAVA;

CommandChangeLayerMaterialProperties::CommandChangeLayerMaterialProperties(ParticleLayer* layer_, const FilePath& spritePath, eBlending blending, bool enableFog, bool enableBlending)
    : RECommand(CMDID_PARTICLE_LAYER_CHANGED_MATERIAL_VALUES, "Change Layer properties")
    , layer(layer_)
{
    newParams.spritePath = spritePath;
    newParams.blending = blending;
    newParams.enableFog = enableFog;
    newParams.enableBlending = enableBlending;

    DVASSERT(layer != nullptr);
    if (layer != nullptr)
    {
        oldParams.spritePath = layer->spritePath;
        oldParams.blending = layer->blending;
        oldParams.enableFog = layer->enableFog;
        oldParams.enableBlending = layer->enableFrameBlend;
    }
}

void CommandChangeLayerMaterialProperties::Redo()
{
    ApplyParams(newParams);
}

void CommandChangeLayerMaterialProperties::Undo()
{
    ApplyParams(oldParams);
}

void CommandChangeLayerMaterialProperties::ApplyParams(const CommandChangeLayerMaterialProperties::LayerParams& params)
{
    if (layer != nullptr)
    {
        layer->SetSprite(params.spritePath);
        layer->blending = params.blending;
        layer->enableFog = params.enableFog;
        layer->enableFrameBlend = params.enableBlending;
    }
}

DAVA::ParticleLayer* CommandChangeLayerMaterialProperties::GetLayer() const
{
    return layer;
}
