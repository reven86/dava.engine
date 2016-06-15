#ifndef __PARTICLE_LAYER_COMMANDS_H__
#define __PARTICLE_LAYER_COMMANDS_H__

#include "Commands2/Base/Command2.h"
#include "Render/RenderBase.h"

namespace DAVA
{
struct ParticleLayer;
class FilePath;
}

class CommandChangeLayerMaterialProperties : public Command2
{
public:
    CommandChangeLayerMaterialProperties(DAVA::ParticleLayer* layer, const DAVA::FilePath& spritePath, DAVA::eBlending blending, bool enableFog, bool enableBlending);

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const override;

    DAVA::ParticleLayer* GetLayer() const;

private:
    struct LayerParams
    {
        DAVA::FilePath spritePath;
        DAVA::eBlending blending = DAVA::BLENDING_NONE;
        bool enableFog = false;
        bool enableBlending = false;
    };

    void ApplyParams(const LayerParams& params);

private:
    LayerParams newParams;
    LayerParams oldParams;
    DAVA::ParticleLayer* layer = nullptr;
};




#endif //__PARTICLE_LAYER_COMMANDS_H__